#include "Serial.h"
#include <algorithm>
#include <cctype>
#include <cstdio>
#include <initguid.h>
#include <devguid.h>
#include <setupapi.h>
#include <string>
#include <tchar.h>
#include <vector>

namespace {
bool extractComIndex(const std::string& text, int& index) {
    for (std::size_t i = 0; i + 3 < text.size(); ++i) {
        if (std::toupper(static_cast<unsigned char>(text[i])) != 'C' ||
            std::toupper(static_cast<unsigned char>(text[i + 1])) != 'O' ||
            std::toupper(static_cast<unsigned char>(text[i + 2])) != 'M') {
            continue;
        }

        std::size_t pos = i + 3;
        if (pos >= text.size() || !std::isdigit(static_cast<unsigned char>(text[pos]))) {
            continue;
        }

        int value = 0;
        while (pos < text.size() && std::isdigit(static_cast<unsigned char>(text[pos]))) {
            value = value * 10 + (text[pos] - '0');
            ++pos;
        }
        if (value > 0) {
            index = value;
            return true;
        }
    }
    return false;
}

std::string lowerCopy(std::string text) {
    std::transform(text.begin(), text.end(), text.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return text;
}

bool containsAny(const std::string& text, const char* const* words, int count) {
    for (int i = 0; i < count; ++i) {
        if (text.find(words[i]) != std::string::npos) {
            return true;
        }
    }
    return false;
}

bool isPreferredUsbSerial(const std::string& name, const std::string& hardwareId) {
    const std::string text = lowerCopy(name + " " + hardwareId);
    static const char* const blockList[] = {
        "bthenum", "bluetooth"
    };
    static const char* const allowList[] = {
        "usb", "ch340", "ch341", "cp210", "ftdi", "ttl", "uart",
        "stlink", "st-link", "vcp", "usbser"
    };
    return containsAny(text, allowList, static_cast<int>(sizeof(allowList) / sizeof(allowList[0]))) &&
           !containsAny(text, blockList, static_cast<int>(sizeof(blockList) / sizeof(blockList[0])));
}

std::string propertyString(HDEVINFO devices, SP_DEVINFO_DATA& data, DWORD property) {
    char buffer[512] = {};
    DWORD type = 0;
    DWORD bytes = 0;
    if (!SetupDiGetDeviceRegistryPropertyA(devices, &data, property, &type,
                                           reinterpret_cast<PBYTE>(buffer),
                                           sizeof(buffer), &bytes)) {
        return {};
    }

    std::string result;
    const DWORD limit = std::min<DWORD>(bytes, sizeof(buffer));
    for (DWORD i = 0; i < limit; ++i) {
        if (buffer[i] == '\0') {
            if (!result.empty() && result.back() != ' ') {
                result.push_back(' ');
            }
            continue;
        }
        result.push_back(buffer[i]);
    }
    while (!result.empty() && result.back() == ' ') {
        result.pop_back();
    }
    return result;
}

void addPort(std::vector<SerialPortInfo>& ports, int index, const std::string& name,
             bool preferred) {
    if (index <= 0) {
        return;
    }
    auto it = std::find_if(ports.begin(), ports.end(), [index](const SerialPortInfo& port) {
        return port.index == index;
    });
    if (it != ports.end()) {
        it->preferred = it->preferred || preferred;
        if (it->name.empty() && !name.empty()) {
            it->name = name;
        }
        return;
    }
    ports.push_back({index, name, preferred});
}

void addRegistryFallbackPorts(std::vector<SerialPortInfo>& ports) {
    for (int i = 1; i <= 256; ++i) {
        char name[16] = {};
        std::snprintf(name, sizeof(name), "COM%d", i);
        char target[256] = {};
        if (QueryDosDeviceA(name, target, sizeof(target)) != 0) {
            addPort(ports, i, name, false);
        }
    }
}

static PortHandle openPort(int idx) {
    TCHAR name[64];
    wsprintf(name, TEXT("\\\\.\\COM%d"), idx);

    PortHandle h = CreateFile(name,
        GENERIC_READ | GENERIC_WRITE,
        0, nullptr, OPEN_EXISTING, 0, nullptr);

    if (h == INVALID_HANDLE_VALUE) return nullptr;

    COMMTIMEOUTS to = {};
    to.ReadIntervalTimeout         = MAXDWORD;
    to.ReadTotalTimeoutConstant    = 20;
    to.ReadTotalTimeoutMultiplier  = 0;
    to.WriteTotalTimeoutConstant   = 50;
    to.WriteTotalTimeoutMultiplier = 10;

    if (!SetCommTimeouts(h, &to)) {
        CloseHandle(h);
        return nullptr;
    }
    return h;
}
}

template<typename F>
static bool withDCB(PortHandle h, F&& fn) {
    DCB dcb = { sizeof(dcb) };
    if (!GetCommState(h, &dcb)) return false;
    fn(dcb);
    return SetCommState(h, &dcb);
}

std::vector<SerialPortInfo> serialListPorts() {
    std::vector<SerialPortInfo> ports;
    HDEVINFO devices = SetupDiGetClassDevsA(&GUID_DEVCLASS_PORTS, nullptr, nullptr,
                                            DIGCF_PRESENT);
    if (devices != INVALID_HANDLE_VALUE) {
        for (DWORD i = 0;; ++i) {
            SP_DEVINFO_DATA data = {};
            data.cbSize = sizeof(data);
            if (!SetupDiEnumDeviceInfo(devices, i, &data)) {
                break;
            }

            std::string name = propertyString(devices, data, SPDRP_FRIENDLYNAME);
            if (name.empty()) {
                name = propertyString(devices, data, SPDRP_DEVICEDESC);
            }
            const std::string hardwareId = propertyString(devices, data, SPDRP_HARDWAREID);

            int index = 0;
            if (extractComIndex(name, index)) {
                addPort(ports, index, name, isPreferredUsbSerial(name, hardwareId));
            }
        }
        SetupDiDestroyDeviceInfoList(devices);
    }

    if (ports.empty()) {
        addRegistryFallbackPorts(ports);
    }

    std::sort(ports.begin(), ports.end(), [](const SerialPortInfo& a, const SerialPortInfo& b) {
        if (a.preferred != b.preferred) {
            return a.preferred && !b.preferred;
        }
        return a.index < b.index;
    });
    return ports;
}

PortHandle serialOpen(int idx, int baud, int bits, int stopbits, int parity, bool logFailure) {
    PortHandle h = openPort(idx);
    if (!h) {
        if (logFailure) {
            printf("open COM%d fail\n", idx);
        }
        return nullptr;
    }
    auto fail = [&](const char* msg) {
        if (logFailure) {
            printf("%s\n", msg);
        }
        CloseHandle(h);
        return nullptr;
    };

    if (!withDCB(h, [&](DCB& d) {
            d.BaudRate = baud;
            d.fBinary = TRUE;
        }))
        return fail("set baud fail");
    if (!withDCB(h, [&](DCB& d) { d.ByteSize = bits; }))
        return fail("set databits fail");
    if (!withDCB(h, [&](DCB& d) { d.StopBits = ONESTOPBIT; }))
        return fail("set stopbits fail");
    if (!withDCB(h, [&](DCB& d) { d.Parity = parity; }))
        return fail("set parity fail");

    PurgeComm(h, PURGE_RXCLEAR | PURGE_TXCLEAR);
    return h;
}

void serialClose(PortHandle h) {
    if (h) {
        CloseHandle(h);
    }
}

int serialSend(PortHandle h, const char* data, int len) {
    DWORD written = 0;
    return WriteFile(h, data, len, &written, nullptr) ? (int)written : -1;
}

int serialRecv(PortHandle h, char* data, int len) {
    DWORD errors = 0;
    COMSTAT stat = {};
    if (!ClearCommError(h, &errors, &stat)) return -1;
    if (errors != 0 && (errors & (CE_FRAME | CE_IOE | CE_OVERRUN | CE_RXOVER)) != 0) {
        ClearCommError(h, &errors, &stat);
    }

    DWORD read = 0;
    const DWORD toRead = std::min<DWORD>(static_cast<DWORD>(len), stat.cbInQue);
    if (toRead == 0) {
        data[0] = '\0';
        return 0;
    }
    if (!ReadFile(h, data, toRead, &read, nullptr)) return -1;
    data[read] = '\0';
    return (int)read;
}

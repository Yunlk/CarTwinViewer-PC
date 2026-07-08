#include "Serial.h"
#include <cstdio>
#include <tchar.h>

static PortHandle openPort(int idx) {
    TCHAR name[64];
    wsprintf(name, TEXT("\\\\.\\COM%d"), idx);

    PortHandle h = CreateFile(name,
        GENERIC_READ | GENERIC_WRITE,
        0, nullptr, OPEN_EXISTING, 0, nullptr);

    if (h == INVALID_HANDLE_VALUE) return nullptr;

    COMMTIMEOUTS to = {};
    to.ReadIntervalTimeout         = 50;
    to.ReadTotalTimeoutConstant    = 50;
    to.ReadTotalTimeoutMultiplier  = 10;
    to.WriteTotalTimeoutConstant   = 50;
    to.WriteTotalTimeoutMultiplier = 10;

    if (!SetCommTimeouts(h, &to) || !SetCommMask(h, EV_RXCHAR)) {
        CloseHandle(h);
        return nullptr;
    }
    return h;
}

template<typename F>
static bool withDCB(PortHandle h, F&& fn) {
    DCB dcb = { sizeof(dcb) };
    if (!GetCommState(h, &dcb)) return false;
    fn(dcb);
    return SetCommState(h, &dcb);
}

PortHandle serialOpen(int idx, int baud, int bits, int stopbits, int parity) {
    PortHandle h = openPort(idx);
    if (!h) {
        printf("open COM%d fail\n", idx);
        return nullptr;
    }
    auto fail = [&](const char* msg) {
        printf("%s\n", msg);
        CloseHandle(h);
        return nullptr;
    };

    if (!withDCB(h, [&](DCB& d) { d.BaudRate = baud; }))
        return fail("set baud fail");
    if (!withDCB(h, [&](DCB& d) { d.ByteSize = bits; }))
        return fail("set databits fail");
    if (!withDCB(h, [&](DCB& d) { d.StopBits = ONESTOPBIT; }))
        return fail("set stopbits fail");
    if (!withDCB(h, [&](DCB& d) { d.Parity = parity; }))
        return fail("set parity fail");

    return h;
}

void serialClose(PortHandle h) {
    CloseHandle(h);
}

int serialSend(PortHandle h, const char* data, int len) {
    DWORD written = 0;
    return WriteFile(h, data, len, &written, nullptr) ? (int)written : -1;
}

int serialRecv(PortHandle h, char* data, int len) {
    DWORD mask = 0, read = 0;
    if (!WaitCommEvent(h, &mask, nullptr)) return -1;
    if (!ReadFile(h, data, len, &read, nullptr)) return -1;
    data[read] = '\0';
    return (int)read;
}

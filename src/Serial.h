#pragma once

#include <windows.h>
#include <string>
#include <vector>
#include "common.h"

using PortHandle = HANDLE;

struct SerialPortInfo {
    int index = 0;
    std::string name;
    bool preferred = false;
};

std::vector<SerialPortInfo> serialListPorts();
PortHandle serialOpen(int idx, int baud, int bits, int stopbits, int parity,
                      bool logFailure = true);
void       serialClose(PortHandle h);
int        serialSend(PortHandle h, const char* data, int len);
int        serialRecv(PortHandle h, char* data, int len);

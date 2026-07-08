#pragma once

#include <windows.h>
#include "common.h"

using PortHandle = HANDLE;

PortHandle serialOpen(int idx, int baud, int bits, int stopbits, int parity);
void       serialClose(PortHandle h);
int        serialSend(PortHandle h, const char* data, int len);
int        serialRecv(PortHandle h, char* data, int len);

#pragma once

#ifndef UNICODE
#define UNICODE
#endif

#include <iostream>
#include <string>
#include <vector>
#include <windows.h>

namespace MonitorManager {

typedef struct {
    std::wstring adapterName, adapterString, adapterID, adapterKey;
    std::wstring monitorName, monitorString, monitorID, monitorKey;
    DWORD stateFlagsAdapter, stateFlagsMonitor;
    UINT ScreenRes[2], WorkArea[2], FullArea[2];
    bool isPrimary;
} monitorData;

struct DataBag {
    bool found;
    HMONITOR hMonitor;
    std::wstring devName;

    DataBag(HMONITOR hmon, LPWSTR name);
};

BOOL monitorEnumProc(HMONITOR hMonitor, HDC hDC, LPRECT rc, LPARAM data);

void getMonitors(std::vector<monitorData> &Monitors);

int getTaskbarHeight(monitorData &Monitor);

} // namespace MonitorManager
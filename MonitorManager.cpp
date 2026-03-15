#include "MonitorManager.h"

MonitorManager::DataBag::DataBag(HMONITOR hmon, LPWSTR name) {
    hMonitor = hmon;
    devName = name;
    found = false;
}

BOOL MonitorManager::monitorEnumProc(HMONITOR hMonitor, HDC hDC, LPRECT rc,
                                     LPARAM data) {
    auto &bag = *reinterpret_cast<DataBag *>(data);
    if (bag.found) return TRUE;

    MONITORINFOEX mi;
    mi.cbSize = sizeof(mi);

    if (GetMonitorInfoW(hMonitor, &mi) &&
        (!wcscmp(mi.szDevice, bag.devName.c_str()))) {
        bag.hMonitor = hMonitor;
        bag.found = true;
    }

    return TRUE;
}

void MonitorManager::getMonitors(std::vector<monitorData> &Monitors) {
    int index = 0;
    DISPLAY_DEVICEW dd = {};
    dd.cb = sizeof dd;
    while (EnumDisplayDevicesW(NULL, index, &dd, 0)) {
        if (dd.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP) {
            monitorData monitor;

            DWORD iMode = 0;
            DEVMODEW devMode = {};
            devMode.dmSize = sizeof(devMode);
            devMode.dmDriverExtra = 100;
            if (EnumDisplaySettingsW(dd.DeviceName, ENUM_CURRENT_SETTINGS,
                                     &devMode)) {
                monitor.adapterID = dd.DeviceID;
                monitor.adapterKey = dd.DeviceKey;
                monitor.adapterName = dd.DeviceName;
                monitor.adapterString = dd.DeviceString;
                monitor.stateFlagsAdapter = dd.StateFlags;

                monitor.ScreenRes[0] = devMode.dmPelsWidth;
                monitor.ScreenRes[1] = devMode.dmPelsHeight;

                DataBag bag(NULL, dd.DeviceName);

                BOOL res = EnumDisplayMonitors(NULL, NULL, monitorEnumProc,
                                               reinterpret_cast<LPARAM>(&bag));

                if ((res && bag.hMonitor) == false) {
                    continue;
                }

                MONITORINFOEX monitorInfo;
                monitorInfo.cbSize = sizeof(monitorInfo);
                if (!GetMonitorInfoW(bag.hMonitor, &monitorInfo)) continue;

                monitor.WorkArea[0] =
                    monitorInfo.rcWork.right - monitorInfo.rcWork.left;
                monitor.WorkArea[1] =
                    monitorInfo.rcWork.bottom - monitorInfo.rcWork.top;

                monitor.FullArea[0] =
                    monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left;
                monitor.FullArea[1] =
                    monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top;

                monitor.isPrimary = (monitorInfo.rcMonitor.top ||
                                     monitorInfo.rcMonitor.left) == false;
            }

            DISPLAY_DEVICE dd1 = {};
            dd1.cb = sizeof(dd1);
            if (EnumDisplayDevicesW(dd.DeviceName, 0, &dd1, 0)) {
                monitor.monitorID = dd1.DeviceID;
                monitor.monitorKey = dd1.DeviceKey;
                monitor.monitorName = dd1.DeviceName;
                monitor.monitorString = dd1.DeviceString;
                monitor.stateFlagsMonitor = dd1.StateFlags;
            } else {
                continue;
            }

            Monitors.push_back(monitor);
        }

        index++;
    }
}

int MonitorManager::getTaskbarHeight(monitorData &Monitor) {
    return Monitor.ScreenRes[1] * (Monitor.FullArea[1] - Monitor.WorkArea[1]) /
           Monitor.FullArea[1];
}
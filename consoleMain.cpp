#ifndef UNICODE
#define UNICODE
#endif

#include <iostream>

#include "MonitorManager.h"
#include "imageHandler.h"
#include "smartPointer.h"

void showMonitorOptions(std::vector<MonitorManager::monitorData> &Monitors) {
    std::cout << "Monitors detected:\n";
    for (int i = 0; i < Monitors.size(); i++) {
        auto &Monitor = Monitors[i];

        std::wstring monitorName(Monitor.monitorName);
        int findLoc = monitorName.find(Monitor.adapterName) +
                      Monitor.adapterName.length();

        if (findLoc != monitorName.npos)
            monitorName =
                monitorName.substr(findLoc, monitorName.length() - findLoc);

        std::wcout << i + 1 << L": " << monitorName;
        std::wcout << L" [" << Monitor.monitorString << L"]";
        if (Monitor.isPrimary) std::cout << " (Primary)";
        std::wcout << L"\t" << Monitor.adapterName << L"\n";

        std::cout << "Dimensions: " << Monitor.ScreenRes[0] << "x"
                  << Monitor.ScreenRes[1] << "\t";

        std::cout << "Taskbar height: "
                  << MonitorManager::getTaskbarHeight(Monitor) << "\n";
    }
}

void showFittingOptions(std::vector<std::string> &fittingOptions) {
    for (auto &option : fittingOptions) {
        std::cout << option;
    }
}

UINT getValidInput() {
    std::string s;
    std::cin >> s;

    if (s.empty()) return -1;

    for (char c : s) {
        if ((c > '9') || (c < '0')) return -1;
    }

    return std::stoi(s);
}

void runApplication() {
    COMHelper::getHResult(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED));

    std::cout << "+-------------------+\n"
                 "| WALLPAPER MANAGER |\n"
                 "+-------------------+\n\n";

    bool continueApp = true;

    auto fittingOptions = ImgHandler::readFittingOptions();

    do {
        std::vector<MonitorManager::monitorData> Monitors;
        MonitorManager::getMonitors(Monitors);

        showMonitorOptions(Monitors);

        std::cout << "Enter monitor index: ";
        UINT monitorIndex = getValidInput();

        while ((monitorIndex < 1) || (monitorIndex > Monitors.size())) {
            std::cout << "Invalid index. Try again: ";
            monitorIndex = getValidInput();
        }

        std::cout << "\nChoose a wallpaper.\n";

        ImgHandler::Img sample(ImgHandler::getWallpaper());

        std::cout << "Fitting options: \n";
        showFittingOptions(fittingOptions);

        std::cout << "Enter fitting option index: ";
        UINT fittingIndex = getValidInput();

        while ((fittingIndex < 1) || (fittingIndex > 4)) {
            std::cout << "Invalid index. Try again: ";
            fittingIndex = getValidInput();
        }

        sample.fitResize(Monitors[monitorIndex - 1],
                         fittingIndex | ImgHandler::AlignBottom);

        std::cout << "Enter Y to continue, or any other key to exit: ";
        std::string continueString;
        std::cin >> continueString;
        if ((continueString != "Y") && (continueString != "y")) {
            continueApp = false;
        }

    } while (continueApp);

    CoUninitialize();
}

int main() {
    runApplication();

    return 0;
}
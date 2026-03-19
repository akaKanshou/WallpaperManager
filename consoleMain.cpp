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

void showFittingOptions(int from, int to) {
    static auto fittingOptions = ImgHandler::readFittingOptions();
    for (int i = from; i <= to; i++) {
        std::cout << fittingOptions[i];
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

        std::cout << "\nChoose a wallpaper: ";

        COMHelper::COFreePointer<LPWSTR> pWallpaperPath(
            ImgHandler::getWallpaper());
        std::wcout << pWallpaperPath.get() << "\n\n";
        ImgHandler::Img sample(pWallpaperPath.get());

        std::cout << "Fitting options: \n";
        showFittingOptions(0, 5);

        std::cout << "Enter fitting option index: ";
        UINT fittingIndex = getValidInput();

        while ((fittingIndex < 1) || (fittingIndex > 4)) {
            std::cout << "Invalid index. Try again: ";
            fittingIndex = getValidInput();
        }

        fittingIndex = 1 << (fittingIndex - 1);

        if (fittingIndex == ImgHandler::Fill) {
            std::cout << "Alignment Options: \n";
            showFittingOptions(6, 8);

            std::cout << "Enter alignment option index: ";
            UINT alignmentOption = getValidInput();

            while ((alignmentOption < 1) || (alignmentOption > 3)) {
                std::cout << "Invalid index. Try again: ";
                alignmentOption = getValidInput();
            }

            if (alignmentOption == 1) {
                fittingIndex |= ImgHandler::AlignTop;
            } else if (alignmentOption == 2) {
                fittingIndex |= ImgHandler::AlignBottom;
            } else {
                fittingIndex |= ImgHandler::AlignCenterVertical;
            }

        } else if (fittingIndex == ImgHandler::Fit) {
            std::cout << "Alignment Options: \n";
            showFittingOptions(9, 11);

            std::cout << "Enter alignment option index: ";
            UINT alignmentOption = getValidInput();

            while ((alignmentOption < 1) || (alignmentOption > 3)) {
                std::cout << "Invalid index. Try again: ";
                alignmentOption = getValidInput();
            }

            if (alignmentOption == 1) {
                fittingIndex |= ImgHandler::AlignLeft;
            } else if (alignmentOption == 2) {
                fittingIndex |= ImgHandler::AlignRight;
            } else {
                fittingIndex |= ImgHandler::AlignCenterHorizontal;
            }
        } else if (fittingIndex == ImgHandler::Strech) {
            std::cout << "Alignment Options: \n";
            showFittingOptions(12, 13);

            std::cout << "Enter alignment option index: ";
            UINT alignmentOption = getValidInput();

            while ((alignmentOption < 1) || (alignmentOption > 2)) {
                std::cout << "Invalid index. Try again: ";
                alignmentOption = getValidInput();
            }

            if (alignmentOption == 1) {
                fittingIndex |= ImgHandler::StrechX;
            } else if (alignmentOption == 2) {
                fittingIndex |= ImgHandler::StrechY;
            }
        }

        if (sample.fitResize(Monitors[monitorIndex - 1], fittingIndex)) {
            COMHelper::COFreePointer<LPWSTR> saveFileLocation(
                ImgHandler::getSaveLocation());
            if (saveFileLocation.get()) {
                if (sample.writeToPng(saveFileLocation.get())) {
                    std::cout << "Operation Successful.\n";
                } else {
                    std::cout << "Operation Failed.\n";
                }
            } else {
                std::cout << "Operation Cancelled\n";
            }
        } else {
            std::cout << "Error: Operation Failed\n";
        }

        std::cout << "\nEnter Y to continue, or any other key to exit: ";
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
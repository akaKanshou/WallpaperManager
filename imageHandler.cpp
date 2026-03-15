#ifndef UNICODE
#define UNICODE
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "imageHandler.h"

#define STBI_WINDOWS_UTF8
#define STB_IMAGE_IMPLEMENTATION

#include "stb_image.h"
#include "stb_image_resize2.h"

void ImgHandler::Img::fitResize(MonitorManager::monitorData &Monitor, int Fit) {
    if (Fit == Fill) {
        /*
        int imageHeight = Monitor.WorkArea[1] * width / Monitor.WorkArea[0];

        RECT copyArea;
        copyArea.left = 0;
        copyArea.right = width - 1;

        if (AlignTop | Fit) {
            copyArea.top = 0;
            copyArea.bottom = imageHeight - 1;
        }

        else if (AlignBottom | Fit) {
            copyArea.top = height - imageHeight;
            copyArea.bottom = height - 1;
        }

        else {
        }

        */

        double scaling = width / Monitor.FullArea[0];
        int taskbarHeight =
            (Monitor.FullArea[1] - Monitor.WorkArea[1]) * scaling;

        double workAreaRatio = Monitor.WorkArea[1] / Monitor.FullArea[1];
        int imageHeight =
            std::ceil(workAreaRatio * taskbarHeight / (1 - workAreaRatio));

        RECT copyArea;
        copyArea.left = 0;
        copyArea.right = width - 1;

        if (Fit | AlignTop) {
        }

        else if (Fit | AlignBottom) {
            copyArea.bottom = height - 1;
            copyArea.top = std::max(height - imageHeight, 0);
        }

        else {
        }

        Img imgWithTBPadding;
        copyRect(imgWithTBPadding, *this, copyArea);
    }
}

ImgHandler::Img::Img() : bitmap(NULL) {}

void ImgHandler::Img::copyRect(Img &target, Img &source, RECT copyArea) {
    target.width = copyArea.right - copyArea.left;
    target.height = copyArea.bottom - copyArea.top;
    target.n = source.n;
    target.bitmap = new unsigned char[target.width * target.height * target.n];
}

ImgHandler::Img::Img(wchar_t *imgPath) {
    size_t len = wcslen(imgPath) + 1;
    char *filename = new char[len];
    stbi_convert_wchar_to_utf8(filename, len, imgPath);
    stbi_load(filename, &width, &height, &n, 0);
    CoTaskMemFree(imgPath);
}

ImgHandler::Img::~Img() {
    if (bitmap) stbi_image_free(bitmap);
}

LPWSTR ImgHandler::getWallpaper() {
    COMHelper::CORleasePointer<IFileDialog> spFileDialogue;
    COMHelper::getHResult(CoCreateInstance(
        CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_IFileOpenDialog,
        reinterpret_cast<void **>(&spFileDialogue)));

    LPWSTR wallpaperPath;

    if (HRESULT error = spFileDialogue->Show(NULL)) {
        COMHelper::CORleasePointer<IDesktopWallpaper> pWallpaper;
        COMHelper::getHResult(CoCreateInstance(
            CLSID_DesktopWallpaper, NULL, CLSCTX_ALL, IID_IDesktopWallpaper,
            reinterpret_cast<void **>(&pWallpaper)));

        COMHelper::COFreePointer<LPWSTR> monitorPath;

        pWallpaper->GetMonitorDevicePathAt(0, &monitorPath);
        pWallpaper->GetWallpaper(monitorPath.get(), &wallpaperPath);

    } else {

        COMHelper::getHResult(error);

        COMHelper::CORleasePointer<IShellItem> pShell;
        COMHelper::getHResult(spFileDialogue->GetResult(&pShell));
        COMHelper::getHResult(
            pShell->GetDisplayName(SIGDN_FILESYSPATH, &wallpaperPath));
    }

    return wallpaperPath;
}

std::vector<std::string> ImgHandler::readFittingOptions() {
    std::fstream file("C:"
                      "\\Users\\sidda\\Desktop\\repo\\winAPI\\WallpaperManager"
                      "\\fittingOptions."
                      "dat",
                      std::ios::in | std::ios::binary);

    std::vector<std::string> result;

    file.seekg(0, file.end);
    int fileEnd = file.tellg();
    file.seekg(0, file.beg);

    while (file.tellg() != fileEnd) {
        int size;
        file.read(reinterpret_cast<char *>(&size), sizeof(size));

        char *s = new char[size];
        file.read(s, size);

        result.emplace_back(s);
    }

    file.close();

    return result;
}

#ifndef UNICODE
#define UNICODE
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "imageHandler.h"

#define STBI_WINDOWS_UTF8
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "stb_image.h"
#include "stb_image_resize2.h"
#include "stb_image_write.h"

void ImgHandler::Img::fitResize(MonitorManager::monitorData &Monitor, int Fit) {
    if (Fit & Fill) {
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

        double scaling = (double)width / Monitor.FullArea[0];
        int taskbarHeight =
            (Monitor.FullArea[1] - Monitor.WorkArea[1]) * scaling;

        int imageHeight;
        if (taskbarHeight) {
            double workAreaRatio =
                (double)(Monitor.FullArea[1] - Monitor.WorkArea[1]) /
                Monitor.WorkArea[1];

            imageHeight = std::ceil(taskbarHeight / workAreaRatio);
        } else {
            imageHeight = scaling * Monitor.FullArea[1];
        }

        // std::cout << width << " " << height << " " << channels << "\n";
        // std::cout << taskbarHeight << " " << imageHeight << " "
        //           << imageHeight + taskbarHeight << "\n";

        int yOffset = 0;

        if (Fit & AlignBottom) {
            yOffset = height - imageHeight;
        }

        else if (Fit & AlignCenterVertical) {
            yOffset = (height - imageHeight) / 2;
        }

        // std::cout << yOffset << "\n";

        Img imgWithTBPadding(
            width, std::ceil(width * Monitor.FullArea[1] / Monitor.FullArea[0]),
            channels);

        if (imgWithTBPadding.createBM(*this, 0, yOffset, taskbarHeight)) {
            // std::cout << "Created BM\n";
            imgWithTBPadding.writeToPng(
                "C:"
                "\\Users\\sidda\\Desktop\\repo\\winAPI\\WallpaperManager"
                "\\output\\output.png");
        }
    }
}

ImgHandler::Img::Img() : bitmap(NULL) {}

ImgHandler::Img::Img(wchar_t *imgPath) {
    size_t len = wcslen(imgPath) + 1;
    char *filename = new char[len];
    stbi_convert_wchar_to_utf8(filename, len, imgPath);
    bitmap = stbi_load(filename, &width, &height, &channels, 0);
    if (!bitmap) std::cerr << "Unable to load wallpaper.";
    CoTaskMemFree(imgPath);
}

ImgHandler::Img::Img(int w, int h, int ch) {
    width = w;
    height = h;
    channels = ch;
    bitmap = (unsigned char *)malloc(w * h * ch);
    memset(bitmap, 0, w * h * ch);
}

ImgHandler::Img::~Img() {
    if (bitmap) stbi_image_free(bitmap);
}

bool ImgHandler::Img::createBM(Img &source, int xOffset, int yOffset,
                               int tbHeight) {

    if (source.channels > channels) std::cerr << "(!) Incomplete copying.\n";
    if (source.channels < channels) {
        std::cerr << "(!) Incompatible images.\n";
        return false;
    }

    for (int a = 0; a < width; a++) {
        for (int b = 0; b < height; b++) {
            copyBit(a, b, source, a + xOffset, b + yOffset);
        }
    }

    return true;
}

void ImgHandler::Img::copyBit(int a, int b, Img &source, int x, int y) {
    if ((x < 0) || (x >= source.width) || (y < 0) || (y >= source.height)) {
        for (int channel = 0; channel < channels; channel++) {
            bitmap[(b * width + a) * channels + channel] = 0;
        }
        return;
    }

    for (int channel = 0; channel < channels; channel++) {
        bitmap[(b * width + a) * channels + channel] =
            source.bitmap[(y * source.width + x) * source.channels + channel];
    }
    return;
}

bool ImgHandler::Img::writeToPng(const char *filename) {
    return stbi_write_png(filename, width, height, channels, (void *)bitmap,
                          width * channels);
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

#ifndef UNICODE
#define UNICODE
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "imageHandler.h"

#define STBI_WINDOWS_UTF8
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "stb_image.h"
#include "stb_image_resize2.h"
#include "stb_image_write.h"

static std::string
    OutputDir("C:"
              "\\Users\\sidda\\Desktop\\repo\\winAPI\\WallpaperManager"
              "\\output\\output.png");

void ImgHandler::Img::fitResize(MonitorManager::monitorData &Monitor, int Fit) {
    if (Fit & Fill) {
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

        int yOffset = 0;

        if (Fit & AlignBottom) {
            yOffset = height - imageHeight;
        }

        else if (Fit & AlignCenterVertical) {
            yOffset = (height - imageHeight) / 2;
        }

        Img imgWithTBPadding(
            width, std::ceil(width * Monitor.FullArea[1] / Monitor.FullArea[0]),
            channels);

        if (imgWithTBPadding.createBM(*this, 0, yOffset)) {
            imgWithTBPadding.writeToPng(OutputDir.c_str());
        }
    }

    else if (Fit & ImgHandler::Fit) {
        double scaling = (double)height / Monitor.WorkArea[1];
        int taskbarHeight =
            (Monitor.FullArea[1] - Monitor.WorkArea[1]) * scaling;

        int imageHeight = height + taskbarHeight,
            imageWidth =
                imageHeight * Monitor.FullArea[0] / Monitor.FullArea[1];

        int xOffset = 0;
        if (Fit & AlignRight) {
            xOffset = width - imageWidth;
        }

        else if (Fit & AlignCenterHorizontal) {
            xOffset = (width - imageWidth) / 2;
        }

        Img imgWithTBPadding(imageWidth, imageHeight, channels);

        if (imgWithTBPadding.createBMResize(*this, xOffset, 0)) {
            imgWithTBPadding.writeToPng(OutputDir.c_str());
        }
    }

    else if (Fit & Strech) {
        if (Fit & StrechY) {
            double scaling = (double)width / Monitor.FullArea[0];
            int imageHeight = std::ceil((double)width * Monitor.FullArea[1] /
                                        Monitor.FullArea[0]),
                taskbarHeight = (Monitor.FullArea[1] - Monitor.WorkArea[1]) *
                                imageHeight / Monitor.FullArea[1];

            Img imgWithTBPadding(width, imageHeight, channels);

            if (imgWithTBPadding.createBMResize(*this, width,
                                                imageHeight - taskbarHeight)) {

                imgWithTBPadding.writeToPng(OutputDir.c_str());
            }
        }

        else {
            double scaling = (double)height / Monitor.WorkArea[1];
            int taskbarHeight =
                (Monitor.FullArea[1] - Monitor.WorkArea[1]) * scaling;

            int imageHeight = height + taskbarHeight,
                imageWidth =
                    imageHeight * Monitor.FullArea[0] / Monitor.FullArea[1];

            Img imgWithTBPadding(imageWidth, imageHeight, channels);

            if (imgWithTBPadding.createBMResize(*this, imageWidth, height)) {
                imgWithTBPadding.writeToPng(OutputDir.c_str());
            }
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

bool ImgHandler::Img::createBM(Img &source, int xOffset, int yOffset) {

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

bool ImgHandler::Img::createBMResize(Img &source, int nWidth, int nHeight) {
    if (source.channels != channels) {
        std::cerr << "(!) Incompatible images.\n";
        return false;
    }

    stbir_pixel_layout layout;
    if (source.channels == 1) {
        layout = STBIR_1CHANNEL;
    } else if (source.channels == 2) {
        layout = STBIR_2CHANNEL;
    } else if (source.channels == 3) {
        layout = STBIR_RGB;
    } else {
        layout = STBIR_RGBA;
    }

    return stbir_resize_uint8_linear(source.bitmap, source.width, source.height,
                                     source.width * source.channels, bitmap,
                                     nWidth, nHeight, channels * nWidth,
                                     layout);
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

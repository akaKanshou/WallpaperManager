#pragma once

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "MonitorManager.h"
#include "smartPointer.h"

namespace ImgHandler {
enum fittingOption {
    Fill = 0x0001,
    Fit = 0x0002,
    Strech = 0x0004,
    Center = 0x0008,
    AlignTop = 0x0010,
    AlignBottom = 0x0020,
    AlignLeft = 0x0040,
    AlignRight = 0x0080,
    AlignCenterVertical = 0x0100,
    AlignCenterHorizontal = 0x0200
};

class Img {
  private:
    unsigned char *bitmap;
    int width, height, channels;

  public:
    Img();
    Img(wchar_t *imgPath);
    Img(int w, int h, int n);
    ~Img();

    void fitResize(MonitorManager::monitorData &Monitor, int Fit);

    bool createBM(Img &source, int xOffset, int yOffset, int tbHeight);

    void copyBit(int a, int b, Img &source, int x, int y);

    bool writeToPng(const char *filename);
};

LPWSTR getWallpaper();

std::vector<std::string> readFittingOptions();
} // namespace ImgHandler

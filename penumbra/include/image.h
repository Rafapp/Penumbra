#pragma once

#include <vector>
#include <cstdint>
#include <iostream>

namespace Image{
    bool LoadImage(const char *filename, std::vector<float>& pixels, int &xres, int &yres, int &nchannels);
}
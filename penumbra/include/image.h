#include <vector>
#include <cstdint>
#include <iostream>

namespace Image{
    bool LoadImage(const char* filename, std::vector<uint8_t>& pixels, int& xres, int& yres, int& nchannels);
}
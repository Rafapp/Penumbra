#include "image.h"
#include <OpenImageIO/imageio.h>

bool Image::LoadImage(const char *filename, std::vector<uint8_t> &pixels, int &xres, int &yres, int &nchannels){
    auto inp = OIIO::ImageInput::open(filename);
    if (!inp) {
        std::cerr << "OIIO open error: " << OIIO::geterror() << std::endl;
        return false;
    }
    const OIIO::ImageSpec& spec = inp->spec();
    xres      = spec.width;
    yres      = spec.height;
    nchannels = spec.nchannels;
    pixels = std::vector<uint8_t>(xres * yres * nchannels, 255);
    bool ok = inp->read_image(0, 0, 0, nchannels, OIIO::TypeDesc::UINT8, pixels.data());
    if (!ok) {
        std::cerr << "OIIO read_image error: " << inp->geterror() << std::endl;
        std::cerr << "Global OIIO error: " << OIIO::geterror() << std::endl;
        inp->close();
        return false;
    }
    inp->close();
    return true;
}
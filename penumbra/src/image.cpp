#include "image.h"

#include <OpenImageIO/imageio.h>

bool Image::LoadImage(const char *filename, std::vector<float>& pixels, int &xres, int &yres, int &nchannels){
    auto inp = OIIO::ImageInput::open(filename); 
    if (!inp) {
        std::cout << "OIIO open error: " << OIIO::geterror() << std::endl;
        return false;
    }
    const OIIO::ImageSpec& spec = inp->spec();
    xres      = spec.width;
    yres      = spec.height;
    nchannels = spec.nchannels;
    pixels = std::vector<float>(xres * yres * nchannels, 0.0f);
    bool ok = inp->read_image(0, 0, 0, nchannels, OIIO::TypeDesc::FLOAT, pixels.data());
    if (!ok) {
        std::cout << "OIIO read_image error: " << inp->geterror() << std::endl;
        std::cout << "Global OIIO error: " << OIIO::geterror() << std::endl;
        inp->close();
        return false;
    }
    inp->close();
    return true;
}
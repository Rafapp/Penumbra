#include <iostream>
#include "main.h"

int main() {
	const char* filename = "/Users/rafa/Downloads/test.png";

	// Test image
	auto inp = OIIO::ImageInput::open(filename);
	if (!inp) {
		std::cerr << "OIIO open error: " << OIIO::geterror() << std::endl;
		return -1;
	}
	const OIIO::ImageSpec& spec = inp->spec();
	int xres      = spec.width;
	int yres      = spec.height;
	int nchannels = spec.nchannels;
	std::vector<uint8_t> pixels(xres * yres * nchannels, 255);
	bool ok = inp->read_image(0, 0, 0, nchannels, OIIO::TypeDesc::UINT8, pixels.data());
	if (!ok) {
		std::cerr << "OIIO read_image error: " << inp->geterror() << std::endl;
		std::cerr << "Global OIIO error: " << OIIO::geterror() << std::endl;
		inp->close();
		return -1;
	}
	inp->close();

	Viewport viewport(960, 540);
	viewport.UpdateTexture(pixels, xres, yres);

    while (!viewport.ShouldClose()) {
        viewport.PollEvents();
		viewport.ShowViewport();
    }

    return 0;
}
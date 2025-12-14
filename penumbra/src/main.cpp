#include <iostream>
#include "main.h"

// TODO: Move to image loading utility file
bool LoadImage(const char* filename, std::vector<uint8_t>& pixels, int& xres, int& yres, int& nchannels) {
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

int main() {
	// Load test image
	// const char* imageFilename = "./resources/images/test.png";
	// int xres, yres, nchannels;
	// std::vector<uint8_t> pixels;
	// if(LoadImage(imageFilename, pixels, xres, yres, nchannels)){
	// 	viewport.UpdateTexture(pixels, xres, yres);
	// }

	// Load PBRT scene
	std::string sceneFilename = "./resources/scenes/cornell.pbrt";
	// std::string sceneFilename = "./resources/scenes/fly_right.pbrt";
	//std::string sceneFilename = "./resources/scenes/cornell_right.pbrt";
	Renderer renderer;
	renderer.LoadScene(sceneFilename);
	
	// Create viewport and display image
	Viewport viewport(&renderer, 960, 540);

    while (!viewport.ShouldClose()) {
		viewport.UpdateTexture(renderer.GetRenderBuffer(), renderer.GetRenderWidth(), renderer.GetRenderHeight());
        viewport.PollEvents();
		viewport.ShowViewport();
    }

    return 0;
}
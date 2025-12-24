#include <iostream>
#include "main.h"

int main() {

	// Load PBRT scene
	std::string sceneFilename = "./resources/scenes/cornell.pbrt";
	// std::string sceneFilename = "./resources/scenes/fly_right.pbrt";
	//std::string sceneFilename = "./resources/scenes/cornell_right.pbrt";
	Renderer renderer;
	renderer.LoadScene(sceneFilename);
	
	// Create viewport and display image
	Viewport viewport(&renderer, 960, 540);


	// Load test image
	// const char* imageFilename = "./resources/images/snooker-room-4k.exr";
	// int xres, yres, nchannels;
	// std::vector<uint8_t> pixels;
	// if(Image::LoadImage(imageFilename, pixels, xres, yres, nchannels)){
		// viewport.UpdateTexture(pixels, xres, yres);
	// }

    while (!viewport.ShouldClose()) {
		viewport.UpdateTexture(renderer.GetRenderBuffer(), renderer.GetRenderWidth(), renderer.GetRenderHeight());
        viewport.PollEvents();
		viewport.ShowViewport();
    }

    return 0;
}
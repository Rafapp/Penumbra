#include <iostream>
#include "main.h"

int main() {
	// Load PBRT scene

	// TODO: Move to GUI, load default scene otherwise
	// std::string sceneFilename = "./resources/scenes/cornell.pbrt";
	std::string sceneFilename = "./resources/scenes/envmap.pbrt";
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
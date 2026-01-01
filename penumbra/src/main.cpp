#include <iostream>
#include "main.h"

int main() {
	// Load PBRT scene
	// std::string sceneFilename = "./resources/scenes/cornell.pbrt";
    // TODO: Better scene file pipeline
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
#include <iostream>
#include "renderer.h"

#ifndef HEADLESS_MODE
#include "gui.h"
#include "viewport.h"
#endif

int main() {
#ifdef HEADLESS_MODE
    std::string sceneFilename = "./resources/scenes/cornell.pbrt";

    Renderer renderer;
    if (!renderer.LoadScene(sceneFilename)) {
        std::cerr << "Failed to load scene: " << sceneFilename << std::endl;
        return 1;
    }

    RenderSettings rs;
    rs.width = 512;
    rs.height = 512;
    rs.spp = 64;
    rs.indirect = true;
    rs.mis = true;
    rs.renderLights = false;
    rs.envMapEnabled = false;
    rs.envMapIntensity = 0.0f;
    rs.gammaCorrect = true;
    rs.tonemap = true;
    rs.exposureBias = 1.0f;

    strncpy(renderer.scenePath, sceneFilename.c_str(), sizeof(renderer.scenePath) - 1);
    renderer.scenePath[sizeof(renderer.scenePath) - 1] = '\0';

    std::string outDir  = "./images/cornell";
    std::string outName = "cornell.png";
    strncpy(renderer.imgOutPath, outDir.c_str(), sizeof(renderer.imgOutPath) - 1);
    renderer.imgOutPath[sizeof(renderer.imgOutPath) - 1] = '\0';
    strncpy(renderer.imgName, outName.c_str(), sizeof(renderer.imgName) - 1);
    renderer.imgName[sizeof(renderer.imgName) - 1] = '\0';

    renderer.RenderHeadless(rs);
    renderer.SaveImage();
    return 0;
#else
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
#endif
}
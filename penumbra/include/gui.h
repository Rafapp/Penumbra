#pragma once

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <functional>
#include <GLFW/glfw3.h>
#include <iostream>

// NOTE: Defaults considered
struct RenderSettings {
    // Rendering
    int width = 960;
    int height = 540;
    int spp = 1;
    bool indirect = true;
    bool mis = true;
    // TODO: Remove this, placeholder for my machines only, or add a file system perhaps
	#if(WIN32)
		char scenePath[256] = "C:\\Users\\rpadi\\Documents\\Dev\\PenumbraDev\\Penumbra\\resources\\scenes\\envmap.pbrt";
		char imgOutPath[256] = "C:\\Users\\rpadi\\Documents\\Dev\\PenumbraDev\\Penumbra\\images\\images";
		char imgName[256] = "out.png";

		// Animation 
		char animPath[256] = "C:\\Users\\rpadi\\Documents\\Dev\\PenumbraDev\\anim\\gen\\envmap";
		char saveAnimPath[256] = "C:\\Users\\rpadi\\Documents\\Dev\\PenumbraDev\\animOut";
	#else
		char scenePath[256] = "/Users/rafa/Documents/Dev/PenumbraDev/Penumbra/build-release/resources/scenes/envmap.pbrt";
		char imgOutPath[256] = "/Users/rafa/Documents/Dev/PenumbraDev/images";
		char imgName[256] = "out.png";

		// Animation 
		char animPath[256] = "/Users/rafa/Documents/Dev/PenumbraDev/Penumbra/resources/scenes/envmap_anim";
		char saveAnimPath[256] = "/Users/rafa/Documents/Dev/PenumbraDev/images/envmap_anim";
	#endif

    // Color
	bool gammaCorrect = true;
	bool tonemap = true;
	float exposureBias = 1.0f;
};

class GUI {
public:
    GUI();
    ~GUI();

    using RenderCallback = std::function<void()>;

    GUI(GLFWwindow* window);

    void NewFrame();
    void Render();
    
    void SetRenderCallback(std::function<void()> callback) {
        renderCallback = callback;
    }
    void SetSaveCallback(std::function<void()> callback) {
        saveCallback = callback;
    }
    void SetRenderAnimCallback(std::function<void()> callback) {
        renderAnimCallback = callback;
    }

    RenderSettings GetRenderSettings() { return renderSettings; }

private:
    std::function<void()> renderCallback;
	std::function<void()> saveCallback;
    std::function<void()> renderAnimCallback;
    GLFWwindow* m_window;
    ImFont* font;
	RenderSettings renderSettings;
    void SetupImGuiStyle();
};
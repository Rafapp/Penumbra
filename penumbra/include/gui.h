#pragma once

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <functional>
#include <GLFW/glfw3.h>
#include <iostream>

struct RenderSettings {
    int width = 960;
    int height = 540;
    int spp = 1;
    bool indirect = true;
    bool mis = true;
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

    RenderSettings GetRenderSettings() { return renderSettings; }

private:
    std::function<void()> renderCallback;
    GLFWwindow* m_window;
    ImFont* font;
	RenderSettings renderSettings;
    void SetupImGuiStyle();
};
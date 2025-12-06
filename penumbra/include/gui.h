#pragma once

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <functional>
#include <GLFW/glfw3.h>
#include <iostream>

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

    int GetRenderWidth() const { return renderWidth; }
    int GetRenderHeight() const { return renderHeight; }

private:
    std::function<void()> renderCallback;
    GLFWwindow* m_window;
    ImFont* font;
    int renderWidth = 960;
    int renderHeight = 540;

    void SetupImGuiStyle();
};
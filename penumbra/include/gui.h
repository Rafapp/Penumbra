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
    
    void SetRenderCallback(RenderCallback callback) { m_renderCallback = callback; }

private:
    GLFWwindow* m_window;
    RenderCallback m_renderCallback;
    ImFont* m_font;

    void SetupImGuiStyle();
};
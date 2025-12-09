#pragma once

#include <glad.h>
#include <vector>
#include <stdexcept>
#include <iostream>
#include <GLFW/glfw3.h>

#include "renderer.h"

class GUI;

class Viewport {
public:
    Viewport(Renderer* renderer, int width, int height);
    ~Viewport();
    void ShowViewport();
    void ResizeViewport(int width, int height);
    bool ShouldClose() const;
    void PollEvents();

    int GetWidth() const { return width; }
    int GetHeight() const { return height; }

    void UpdateTexture(const std::vector<uint8_t>& pixels, int w, int h);

    static void ResizeCallbackStatic(GLFWwindow* window, int w, int h);

    std::vector<uint8_t>& GetWindowBuffer(){ return windowBuffer; }
    
private:
    GLFWwindow* window;
    int width = 0;
    int height = 0;
    int viewportRenderWidth = 0;
    int viewportRenderHeight = 0;
    std::vector<uint8_t> windowBuffer;
    GLuint texture = 0;
    GLuint program = 0;
    GLuint vao = 0;
    GLuint vbo;
    GLuint uniformTexLocation = 0;
    GLint uniformScaleLocation = 0;
    float scaleX = 1.0f, m_scaleY = 1.0f;
    int texWidth = 0;
    int texHeight = 0;

    GLuint compileShader(const char* source, GLenum type);
    void createShaderProgram();

    std::unique_ptr<GUI> gui;
};
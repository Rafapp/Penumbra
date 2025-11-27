#include <glad.h>
#include <vector>
#include <stdexcept>
#include <iostream>
#include <GLFW/glfw3.h>

class Viewport{
public:
    Viewport(int width, int height);
    ~Viewport();
    void ShowViewport();
    void ResizeViewport(int width, int height);
    bool ShouldClose() const;
    void PollEvents();

    int GetWidth() const { return m_width; }
    int GetHeight() const { return m_height; }

    void UpdateTexture(const std::vector<uint8_t>& pixels, int w, int h);

    static void ResizeCallbackStatic(GLFWwindow* window, int w, int h);
    void ResizeCallback(GLFWwindow* window, int w, int h);

    std::vector<uint8_t>& GetWindowBuffer(){ return m_windowBuffer; }

	// TODO: Get these from the pathtracer resolution
    int imgW = 1280;
    int imgH = 720;
    
private:
    GLFWwindow* m_window;
    int m_width, m_height;
    std::vector<uint8_t> m_windowBuffer;
    GLuint m_texture = 0;
    GLuint m_program = 0;
    GLuint m_vao = 0;
    GLuint m_vbo;
    GLuint m_uniformTexLocation = 0;
    GLint m_uniformScaleLocation = 0;
    float m_scaleX = 1.0f, m_scaleY = 1.0f;

    GLuint compileShader(const char* source, GLenum type);
    void createShaderProgram();
};
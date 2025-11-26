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
    void UpdateViewport(int width, int height);
    bool ShouldClose() const;
    void PollEvents();

    std::vector<uint8_t>& GetWindowBuffer(){ return m_windowBuffer; }

private:
    GLFWwindow* m_window;
    int m_width, m_height;
    std::vector<uint8_t> m_windowBuffer;
    GLuint m_texture = 0;
    GLuint m_program = 0;
    GLuint m_vao = 0;
    GLuint m_vbo;
    GLuint m_uniformTexLocation = 0;

    GLuint compileShader(const char* source, GLenum type);
    void createShaderProgram();
};
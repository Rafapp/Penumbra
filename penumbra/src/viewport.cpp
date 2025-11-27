#include "viewport.h"

const char* vertexShader = R"(
#version 150

in vec2 position;
in vec2 texCoord;
out vec2 TexCoord;

uniform vec2 uScale;

void main() {
    vec2 scaled = position * uScale;
    gl_Position = vec4(scaled, 0.0, 1.0);
    TexCoord = texCoord;
}
)";

const char* fragmentShader = R"(
#version 150
in vec2 TexCoord;
out vec4 FragColor;
uniform sampler2D tex;

void main() {
    FragColor = texture(tex, vec2(TexCoord.x, 1.0 - TexCoord.y));
}
)";

GLuint Viewport::compileShader(const char* source, GLenum type) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    
    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "Shader compilation failed: " << infoLog << std::endl;
    }
    return shader;
}

void Viewport::createShaderProgram() {
    GLuint vs = compileShader(vertexShader, GL_VERTEX_SHADER);
    GLuint fs = compileShader(fragmentShader, GL_FRAGMENT_SHADER);

    m_program = glCreateProgram();
    glAttachShader(m_program, vs);
    glAttachShader(m_program, fs);

    glBindAttribLocation(m_program, 0, "position");
    glBindAttribLocation(m_program, 1, "texCoord");

    glLinkProgram(m_program);

    int success;
    char infoLog[512];
    glGetProgramiv(m_program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(m_program, 512, nullptr, infoLog);
        std::cerr << "Program linking failed: " << infoLog << std::endl;
    }

    glDeleteShader(vs);
    glDeleteShader(fs);

    glUseProgram(m_program);
    m_uniformTexLocation = glGetUniformLocation(m_program, "tex");
    glUniform1i(m_uniformTexLocation, 0);
    glUseProgram(0);
}

Viewport::Viewport(int width, int height) : 
    m_width(width), m_height(height) {

    if (!glfwInit()) {
        throw std::runtime_error("Failed to initialize GLFW");
    }
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    
    m_window = glfwCreateWindow(m_width, m_height, "Penumbra", NULL, NULL);
    if(!m_window){
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window");
    }
    
    glfwMakeContextCurrent(m_window);
    int fbw, fbh;
    glfwGetFramebufferSize(m_window, &fbw, &fbh);
    m_width  = fbw;
    m_height = fbh;

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        throw std::runtime_error("Failed to load OpenGL functions");
    }

    // TODO: Get these from the pathtracer resolution and allow update
    m_windowBuffer.resize(imgW * imgH * 3, 0);

    glfwSwapInterval(1);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    
    createShaderProgram();

    // Generate and bind texture 
    glGenTextures(1, &m_texture);
    glBindTexture(GL_TEXTURE_2D, m_texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage2D(
        GL_TEXTURE_2D, 0, GL_RGB,
        imgW, imgH,
        0, GL_RGB, GL_UNSIGNED_BYTE,
        nullptr
    );

    glBindTexture(GL_TEXTURE_2D, 0);

    float vertices[] = {
        // First triangle
        -1.f,  1.f,   0.f, 1.f,
        -1.f, -1.f,   0.f, 0.f,
        1.f, -1.f,   1.f, 0.f,

        // Second triangle
        -1.f,  1.f,   0.f, 1.f,
        1.f, -1.f,   1.f, 0.f,
        1.f,  1.f,   1.f, 1.f
    };
    
    // Bind and set up VAO and VBO
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    // Set initial scale uniform
    m_uniformScaleLocation = glGetUniformLocation(m_program, "uScale");
    glUseProgram(m_program);
    glUniform2f(m_uniformScaleLocation, 1.0f, 1.0f);

    // Callbacks
    glfwSetWindowUserPointer(m_window, this);
    glfwSetFramebufferSizeCallback(m_window, ResizeCallbackStatic);

    glBindVertexArray(0);
}

void Viewport::UpdateTexture(const std::vector<uint8_t>& pixels, int w, int h){
    glBindTexture(GL_TEXTURE_2D, m_texture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0,
                imgW, imgH,
                GL_RGB, GL_UNSIGNED_BYTE,
                pixels.data());
}

void Viewport::ShowViewport(){
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(m_program);
    
    // Scale viewport to maintain aspect ratio
    float windowAspect = (float) m_width / m_height;
    float imageAspect  = (float) imgW / imgH;
    float scaleX = (windowAspect > imageAspect) ? (imageAspect / windowAspect) : 1.f;
    float scaleY = (windowAspect > imageAspect) ? 1.f : (windowAspect / imageAspect);
    glUniform2f(m_uniformScaleLocation, scaleX, scaleY);
    glUniform1i(m_uniformTexLocation, 0);
    
    // Bind texture and draw
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_texture);

    glBindVertexArray(m_vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    glfwSwapBuffers(m_window);
}

void Viewport::ResizeViewport(int width, int height){
    m_width = width;
    m_height = height;
    glViewport(0, 0, m_width, m_height);
}

void Viewport::ResizeCallbackStatic(GLFWwindow* window, int w, int h){
    Viewport* vp = static_cast<Viewport*>(glfwGetWindowUserPointer(window));
    if (vp) {
        vp->ResizeCallback(window, w, h);
    }
}

void Viewport::ResizeCallback(GLFWwindow* window, int w, int h){
    m_width = w;
    m_height = h;
    ShowViewport();
}

bool Viewport::ShouldClose() const {
    return glfwWindowShouldClose(m_window);
}

void Viewport::PollEvents(){
    glfwPollEvents();
}

Viewport::~Viewport(){
    if (m_texture != 0) {
        glDeleteTextures(1, &m_texture);
    }
    if (m_window) {
        glfwDestroyWindow(m_window);
    }
    glfwTerminate();
}
#include "viewport.h"

const char* vertexShader = R"(
#version 150
in vec2 position;
in vec2 texCoord;
out vec2 TexCoord;

void main() {
    gl_Position = vec4(position, 0.0, 1.0);
    TexCoord = vec2(texCoord.x, 1.0 - texCoord.y);
}
)";

const char* fragmentShader = R"(
#version 150
in vec2 TexCoord;
out vec4 FragColor;
uniform sampler2D tex;

void main() {
    FragColor = texture(tex, TexCoord);
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

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        throw std::runtime_error("Failed to load OpenGL functions");
    }

    // set viewport to the initial framebuffer size
    int fbw, fbh;
    glfwGetFramebufferSize(m_window, &fbw, &fbh);
    glViewport(0, 0, fbw, fbh);

    // create the texture once (empty)
    glGenTextures(1, &m_texture);
    glBindTexture(GL_TEXTURE_2D, m_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // allocate storage once (null data)
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_width, m_height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glBindTexture(GL_TEXTURE_2D, 0);

    glfwSwapInterval(1);
    m_windowBuffer.resize(m_width * m_height * 3, 0);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    
    createShaderProgram();
    
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
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                          (void*)(2 * sizeof(float)));

    glBindVertexArray(0);
}

void Viewport::ShowViewport(){
    glClear(GL_COLOR_BUFFER_BIT);
    
    // Create/update texture
    if (m_texture == 0) {
        glGenTextures(1, &m_texture);
        glBindTexture(GL_TEXTURE_2D, m_texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_width, m_height, 0, GL_RGB, GL_UNSIGNED_BYTE, m_windowBuffer.data());
    } else {
        glBindTexture(GL_TEXTURE_2D, m_texture);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_width, m_height, GL_RGB, GL_UNSIGNED_BYTE, m_windowBuffer.data());
    }

    glUseProgram(m_program);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_texture);
    glUniform1i(glGetUniformLocation(m_program, "tex"), 0);
    glBindVertexArray(m_vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    glfwSwapBuffers(m_window);
    
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
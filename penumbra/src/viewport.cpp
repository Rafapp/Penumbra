#include "viewport.h"
#include "gui.h"

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

    program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);

    glBindAttribLocation(program, 0, "position");
    glBindAttribLocation(program, 1, "texCoord");

    glLinkProgram(program);

    int success;
    char infoLog[512];
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        std::cerr << "Program linking failed: " << infoLog << std::endl;
    }

    glDeleteShader(vs);
    glDeleteShader(fs);

    glUseProgram(program);
    uniformTexLocation = glGetUniformLocation(program, "tex");
    glUniform1i(uniformTexLocation, 0);
    glUseProgram(0);
}

Viewport::Viewport(Renderer* renderer, int width, int height) : 
    width(width), height(height) {

    if (!glfwInit()) {
        throw std::runtime_error("Failed to initialize GLFW");
    }
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    
    window = glfwCreateWindow(width, height, "Penumbra", NULL, NULL);
    if(!window){
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window");
    }
    
    glfwMakeContextCurrent(window);

    // GUI
    gui = std::make_unique<GUI>(window);
    Renderer* r = renderer;
    gui->SetRenderCallback([this, r]() {
        // TODO: Better scene file pipeline
        auto rs = gui->GetRenderSettings();
        strncpy(r->scenePath, rs.scenePath, sizeof(r->scenePath) - 1);
        r->scenePath[sizeof(r->scenePath) - 1] = '\0';
        r->BeginRender();
    });
    gui->SetSaveCallback([this, r]() {
        auto rs = gui->GetRenderSettings();
        strncpy(r->imgOutPath, rs.imgOutPath, sizeof(r->imgOutPath) - 1);
        r->imgOutPath[sizeof(r->imgOutPath) - 1] = '\0';
        strncpy(r->imgName, rs.imgName, sizeof(r->imgName) - 1);
        r->imgName[sizeof(r->imgName) - 1] = '\0';
        r->SaveImage();
	});
    gui->SetRenderAnimCallback([r]() {
        std::thread([r]() { r->RenderAnimation(); }).detach();
    });
    renderer->SetGUI(gui.get());
	auto rs = gui->GetRenderSettings();
    viewportRenderWidth = rs.width;
    viewportRenderHeight = rs.height;
    int fbw, fbh;
    glfwGetFramebufferSize(window, &fbw, &fbh);
    width  = fbw;
    height = fbh;

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        throw std::runtime_error("Failed to load OpenGL functions");
    }

    windowBuffer.resize(viewportRenderWidth * viewportRenderHeight * 3, 0);

    glfwSwapInterval(1);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    
    createShaderProgram();

    // Generate and bind texture 
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage2D(
        GL_TEXTURE_2D, 0, GL_RGB,
        viewportRenderWidth, viewportRenderHeight, 
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
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    // Set initial scale uniform
    uniformScaleLocation = glGetUniformLocation(program, "uScale");
    glUseProgram(program);
    glUniform2f(uniformScaleLocation, 1.0f, 1.0f);

    // Callbacks
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, ResizeCallbackStatic);

    glBindVertexArray(0);
}

void Viewport::UpdateTexture(const std::vector<uint8_t>& pixels, int w, int h){
 glBindTexture(GL_TEXTURE_2D, texture);

    if (w != texWidth || h != texHeight) {
        texWidth = w;
        texHeight = h;
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8,
                     w, h, 0,
                     GL_RGB, GL_UNSIGNED_BYTE,
                     pixels.data());
    }
    else {
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0,
                        w, h,
                        GL_RGB, GL_UNSIGNED_BYTE,
                        pixels.data());
    }
}

void Viewport::ShowViewport(){
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(program);
    
    // Scale viewport to maintain aspect ratio
    float windowAspect = (float) width / height;
    float imageAspect  = (float) viewportRenderWidth / viewportRenderHeight;
    float scaleX = (windowAspect > imageAspect) ? (imageAspect / windowAspect) : 1.f;
    float scaleY = (windowAspect > imageAspect) ? 1.f : (windowAspect / imageAspect);
    glUniform2f(uniformScaleLocation, scaleX, scaleY);
    glUniform1i(uniformTexLocation, 0);
    
    // Bind texture and draw
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
    gui->NewFrame();
    gui->Render();

    glfwSwapBuffers(window);
}

void Viewport::ResizeCallbackStatic(GLFWwindow* window, int w, int h){
#ifdef __APPLE__ 
    Viewport* vp = static_cast<Viewport*>(glfwGetWindowUserPointer(window));
    if (vp) {
		vp->ResizeViewport(w, h);
    }
#elif _WIN32 
    Viewport* vp = static_cast<Viewport*>(glfwGetWindowUserPointer(window));
    if (vp) {
        // Get actual framebuffer size, not window size
        int fbw, fbh;
        glfwGetFramebufferSize(window, &fbw, &fbh);
        vp->ResizeViewport(fbw, fbh);
    }
#endif
}

void Viewport::ResizeViewport(int width, int height){
    this->width = width;
    this->height = height;
    glViewport(0, 0, width, height);
    ShowViewport();
}

bool Viewport::ShouldClose() const {
    return glfwWindowShouldClose(window);
}

void Viewport::PollEvents(){
    glfwPollEvents();
}

Viewport::~Viewport(){
    if (texture != 0) {
        glDeleteTextures(1, &texture);
    }
    if (window) {
        glfwDestroyWindow(window);
    }
    glfwTerminate();
}
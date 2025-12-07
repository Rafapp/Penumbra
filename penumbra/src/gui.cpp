#include "gui.h"

GUI::GUI(GLFWwindow* window) : m_window(window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();
    SetupImGuiStyle();

    ImGui_ImplGlfw_InitForOpenGL(m_window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // Load font
    ImFontConfig cfg;
    cfg.OversampleH = 5;
    cfg.OversampleV = 5;
    cfg.PixelSnapH = true;
    cfg.RasterizerMultiply = 1.5f;
    font = io.Fonts->AddFontFromFileTTF("./resources/fonts/JetBrainsMono-Regular.ttf", 16.0f, &cfg);
    if (!font) {
        std::cerr << "Failed to load JetBrainsMono-Regular.ttf\n";
    }
    io.Fonts->Build();
}

GUI::~GUI() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void GUI::NewFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void GUI::Render() {
    ImGuiIO& io = ImGui::GetIO();

    // Set position and size
    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_Always);

    // Collapsible window
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoMove;

    if (ImGui::Begin("Render Controls", nullptr, window_flags)) {

        if (font) ImGui::PushFont(font);

        float panelWidth = ImGui::GetContentRegionAvail().x;
        io.FontGlobalScale = 0.005f * panelWidth;

        // Render button
        if (ImGui::Button("Render", ImVec2(panelWidth, 40.0f))) {
            if (renderCallback) {
                renderCallback();
            }
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // Settings menu
        if (ImGui::CollapsingHeader("Settings")) {
            ImGui::Indent(20.0f);

            // Resolution
            if (ImGui::CollapsingHeader("Resolution")) {
                ImGui::Text("Render Width");
                ImGui::SetNextItemWidth(150.0f);
                ImGui::InputInt("##RenderWidth", &renderWidth, 0, 0);
                ImGui::Text("Render Height");
                ImGui::SetNextItemWidth(150.0f);
                ImGui::InputInt("##RenderHeight", &renderHeight, 0, 0);
            }

            ImGui::Unindent(20.0f);

            ImGui::Indent(20.0f);
            // Rendering 
            if (ImGui::CollapsingHeader("Rendering")) {
                ImGui::Text("Samples per Pixel");
                ImGui::SetNextItemWidth(150.0f);
                ImGui::InputInt("##SamplesPerPixel", &spp, 0, 0);
                ImGui::Text("Direct Lighting");
                ImGui::SetNextItemWidth(150.0f);
                ImGui::Checkbox("##Direct Lighting", &directLighting);
                ImGui::Text("Indirect Lighting");
                ImGui::SetNextItemWidth(150.0f);
                ImGui::Checkbox("##Indirect Lighting", &indirectLighting);
            }
            ImGui::Unindent(20.0f);
        }

        if (font) ImGui::PopFont();

    }

    ImGui::End();

    // Render ImGui
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void GUI::SetupImGuiStyle() {
    ImGuiStyle& style = ImGui::GetStyle();

    // Window/frame style
    style.WindowRounding  = 0.0f;
    style.FrameRounding   = 4.0f;
    style.WindowPadding   = ImVec2(10, 10);
    style.FramePadding    = ImVec2(6, 4);
    style.ItemSpacing     = ImVec2(10, 8);
    style.ButtonTextAlign = ImVec2(0.5f, 0.5f);

    // Black theme colors
    ImVec4* colors = style.Colors;
    colors[ImGuiCol_WindowBg]         = ImVec4(0.05f, 0.05f, 0.05f, 1.0f);
    colors[ImGuiCol_FrameBg]          = ImVec4(0.15f, 0.15f, 0.15f, 1.0f);
    colors[ImGuiCol_FrameBgHovered]   = ImVec4(0.25f, 0.25f, 0.25f, 1.0f);
    colors[ImGuiCol_FrameBgActive]    = ImVec4(0.35f, 0.35f, 0.35f, 1.0f);
    colors[ImGuiCol_Button]           = ImVec4(0.2f, 0.2f, 0.2f, 1.0f);
    colors[ImGuiCol_ButtonHovered]    = ImVec4(0.3f, 0.3f, 0.3f, 1.0f);
    colors[ImGuiCol_ButtonActive]     = ImVec4(0.45f, 0.45f, 0.45f, 1.0f);
    colors[ImGuiCol_Text]             = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    colors[ImGuiCol_TitleBg]          = ImVec4(0.05f, 0.05f, 0.05f, 1.0f);
    colors[ImGuiCol_TitleBgActive]    = ImVec4(0.15f, 0.15f, 0.15f, 1.0f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
    colors[ImGuiCol_Header]           = ImVec4(0.2f, 0.2f, 0.2f, 1.0f);
    colors[ImGuiCol_HeaderHovered]    = ImVec4(0.3f, 0.3f, 0.3f, 1.0f);
    colors[ImGuiCol_HeaderActive]     = ImVec4(0.45f, 0.45f, 0.45f, 1.0f);
}
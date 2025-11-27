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
        float panelWidth = ImGui::GetContentRegionAvail().x;
        io.FontGlobalScale = 0.005f * panelWidth;

        // Render button
        if (ImGui::Button("Render", ImVec2(panelWidth, 40.0f))) {
            if (m_renderCallback) {
                m_renderCallback();
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
                static float renderW = 1920.0f;
                ImGui::Text("Render Width");
                ImGui::SetNextItemWidth(150.0f);
                ImGui::InputFloat("##RenderWidth", &renderW);

                static float renderH = 1080.0f;
                ImGui::Text("Render Height");
                ImGui::SetNextItemWidth(150.0f);
                ImGui::InputFloat("##RenderHeight", &renderH);
            }

            ImGui::Unindent(20.0f);
        }
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
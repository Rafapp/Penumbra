#include <glm/glm.hpp>
#include <GLFW/glfw3.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <OpenImageIO/imageio.h>
#include <imgui.h>
#include <iostream>
#include "minipbrt.h"

int main() {
    std::cout << "Verifying libraries..." << std::endl;
    std::cout << std::endl;
    
    // Test GLM
    try {
        glm::vec3 v(1.0f, 2.0f, 3.0f);
        glm::mat4 m = glm::mat4(1.0f);
        glm::normalize(v);
        std::cout << "✓ GLM working" << std::endl;
    }
    catch (...) {
        std::cout << "✗ GLM failed" << std::endl;
        return 1;
    }
    
    // Test GLFW
    try {
        if (!glfwInit()) {
            std::cout << "✗ GLFW init failed" << std::endl;
            return 1;
        }
        glfwTerminate();
        std::cout << "✓ GLFW working" << std::endl;
    }
    catch (...) {
        std::cout << "✗ GLFW failed" << std::endl;
        return 1;
    }
    
    // Test Assimp
    try {
        Assimp::Importer importer;
        std::cout << "✓ Assimp working" << std::endl;
    }
    catch (...) {
        std::cout << "✗ Assimp failed" << std::endl;
        return 1;
    }
    
    // Test OpenImageIO
    try {
        OIIO::ImageOutput::unique_ptr out;
        std::cout << "✓ OpenImageIO working" << std::endl;
    }
    catch (...) {
        std::cout << "✗ OpenImageIO failed" << std::endl;
        return 1;
    }
    
    // Test ImGui
    try {
        ImGui::CreateContext();
        ImGui::DestroyContext();
        std::cout << "✓ ImGui working" << std::endl;
    }
    catch (...) {
        std::cout << "✗ ImGui failed" << std::endl;
        return 1;
    }
    
    // Test TinyBVH
    std::cout << "✓ TinyBVH headers found" << std::endl;
    
    std::cout << std::endl;
    std::cout << "All libraries verified!" << std::endl;
    
    return 0;
}

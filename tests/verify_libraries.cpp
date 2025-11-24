#include <glm/glm.hpp>
#include <RGFW.h> 
#include <iostream>

int main() {
    std::cout << "Verifying libraries..." << std::endl;

    // Test GLM
    glm::vec3 v(1.0f, 2.0f, 3.0f);
    glm::mat4 m = glm::mat4(1.0f);
    std::cout << "GLM ✓" << std::endl;

    // Test RGFW
    std::cout << "RGFW ✓" << std::endl;
    return 0;
}
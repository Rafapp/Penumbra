#include "environmentmap.h"

EnvironmentMap::EnvironmentMap(std::string filepath) {
    this->filepath = filepath;
}

bool EnvironmentMap::Load(){
    if(!Image::LoadImage(filepath.c_str(), image, width, height, nChannels)){
        std::cerr << "Failed to load environment map image: " << filepath << std::endl; 
        return false;
    }
    return true;
}

glm::vec3 EnvironmentMap::SampleColor(const Ray& ray) {
    glm::vec3 d = ray.d;
    float phi = atan2f(d.y, d.x);
    float theta = glm::acos(glm::clamp(d.z, -1.0f, 1.0f)); 
    float u = (phi + M_PI) * M_1_2PI;
    float v = theta * M_1_PI;
    int i = (int(v * (height - 1)) * width + int(u * (width - 1))) * nChannels;
    glm::vec3 color = glm::vec3(image[i], image[i + 1], image[i + 2]);
    return color;
}

EnvironmentMap::~EnvironmentMap() {
}
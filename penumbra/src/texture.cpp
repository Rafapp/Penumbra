#include "texture.h"

// #include "materials.h"

Texture::Texture() {}
Texture::~Texture() {}

bool Texture::Load(const std::string& path){
    this->path = path;

    // Load image data
    if(!Image::LoadImage(path.c_str(), pixels, xres, yres, nchannels)){
        std::cerr << "Failed to load texture: " << path << std::endl;
        return false;
    }
    return true;
}

glm::vec3 Texture::Sample(const glm::vec2& uv) const {
    // Sample the texture at the given UV coordinates
    if (pixels.empty()) return glm::vec3(0.0f);

    // Convert UV coordinates to pixel coordinates
    int x = static_cast<int>(uv.x * xres);
    int y = static_cast<int>((1.0f - uv.y) * yres);
    if (x < 0 || x >= xres || y < 0 || y >= yres) return glm::vec3(0.0f);

    // Get the pixel color
    int index = (y * xres + x) * nchannels;
    if (index + 2 >= pixels.size()) return glm::vec3(0.0f);
    return glm::vec3(pixels[index], pixels[index + 1], pixels[index + 2]);
}
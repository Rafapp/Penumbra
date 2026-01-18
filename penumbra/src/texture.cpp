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

#pragma once

#define _USE_MATH_DEFINES
#include <cmath>

#ifndef M_PI 
#define M_PI 3.14159274101257324219f
#endif
#ifndef M_1_PI
#define M_1_PI 0.31830987334251403809f
#endif

#include <assimp/material.h>
#include "glm/glm.hpp"
#include "sampling.h"
#include "image.h"

std::string TextureTypeToString(aiTextureType type) {
    switch (type) {
    case aiTextureType_DIFFUSE: return "DIFFUSE";
    case aiTextureType_SPECULAR: return "SPECULAR";
    case aiTextureType_AMBIENT: return "AMBIENT";
    case aiTextureType_EMISSIVE: return "EMISSIVE";
    case aiTextureType_HEIGHT: return "HEIGHT";
    case aiTextureType_NORMALS: return "NORMALS";
    case aiTextureType_SHININESS: return "SHININESS";
    case aiTextureType_OPACITY: return "OPACITY";
    case aiTextureType_DISPLACEMENT: return "DISPLACEMENT";
    case aiTextureType_LIGHTMAP: return "LIGHTMAP";
    case aiTextureType_REFLECTION: return "REFLECTION";
    case aiTextureType_BASE_COLOR: return "BASE_COLOR";
    case aiTextureType_NORMAL_CAMERA: return "NORMAL_CAMERA";
    case aiTextureType_EMISSION_COLOR: return "EMISSION_COLOR";
    case aiTextureType_METALNESS: return "METALNESS";
    case aiTextureType_DIFFUSE_ROUGHNESS: return "DIFFUSE_ROUGHNESS";
    case aiTextureType_AMBIENT_OCCLUSION: return "AMBIENT_OCCLUSION";
    case aiTextureType_SHEEN: return "SHEEN";
    case aiTextureType_CLEARCOAT: return "CLEARCOAT";
    case aiTextureType_TRANSMISSION: return "TRANSMISSION";
    case aiTextureType_NONE:
    default: return "NONE";
    }
}
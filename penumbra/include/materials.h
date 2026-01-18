#pragma once

#include<iostream>

#include "glm/glm.hpp"
#include "minipbrt.h"

#include "texture.h"

class Material {
public:
    virtual ~Material() = default;
    
    minipbrt::MaterialType GetType() const { return type; }
    
protected:
    minipbrt::MaterialType type;
};

class MatteMaterial : public Material {
public:
    MatteMaterial(minipbrt::MatteMaterial* pbrtMat);
    ~MatteMaterial() = default;
    
    glm::vec3 albedo;
};

class DisneyMaterial : public Material {
public:
    DisneyMaterial(minipbrt::DisneyMaterial* pbrtMat);
    ~DisneyMaterial() = default;

    // Textured parameters
    ::Texture* albedoTexture = nullptr;
    ::Texture* roughnessTexture = nullptr;
    ::Texture* metallicTexture = nullptr;

    // Bump mapping textures
	::Texture* normalTexture = nullptr;

    // Mesh material parameters
    glm::vec3 albedo = glm::vec3(255.0f, 0.0f, 255.0f);
    float roughness = 0.5f;
    float metallic = 0.0f;
    float eta = 1.0f;

    // Sample textures if available, otherwise return base values
    glm::vec3 GetAlbedo(const glm::vec2& uv) const {
        if (albedoTexture) return albedoTexture->Sample(uv);
        return albedo;
    }
    
    float GetRoughness(const glm::vec2& uv) const {
        if (roughnessTexture) return roughnessTexture->Sample(uv).r;
        return roughness;
    }
    
    float GetMetallic(const glm::vec2& uv) const {
        if (metallicTexture) return metallicTexture->Sample(uv).r;
        return metallic;
    }
};
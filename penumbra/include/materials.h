#pragma once

#include "glm/glm.hpp"
#include "minipbrt.h"

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
    
    glm::vec3 GetAlbedo() const { return albedo; }
    
private:
    glm::vec3 albedo;
};
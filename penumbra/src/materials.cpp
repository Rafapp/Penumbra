#include "materials.h"

MatteMaterial::MatteMaterial(minipbrt::MatteMaterial* pbrtMat) {
    if (!pbrtMat) return;
    type = pbrtMat->type();
    albedo = glm::vec3(pbrtMat->Kd.value[0], pbrtMat->Kd.value[1], pbrtMat->Kd.value[2]); 
}
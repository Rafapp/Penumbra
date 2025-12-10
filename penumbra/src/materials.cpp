#include "materials.h"

MatteMaterial::MatteMaterial(minipbrt::MatteMaterial* pbrtMat) {
    if (!pbrtMat) return;
    type = pbrtMat->type();
    albedo = glm::vec3(pbrtMat->Kd.value[0], pbrtMat->Kd.value[1], pbrtMat->Kd.value[2]); 
}

DisneyMaterial::DisneyMaterial(minipbrt::DisneyMaterial* pbrtMat) {
    // TODO: Access texture params
	std::cout << "Creating DisneyMaterial from PBRT material" << std::endl;
    if (!pbrtMat) return;
    type = pbrtMat->type();
    albedo = glm::vec3(pbrtMat->color.value[0], pbrtMat->color.value[1], pbrtMat->color.value[2]);
    roughness = pbrtMat->roughness.value;
    metallic = pbrtMat->metallic.value;
    eta = pbrtMat->eta.value;
}
#include "renderer.h"

#include "pbrtloader.h"
#include "shading.h"

Renderer::Renderer() {
    scene = std::make_unique<Scene>();
    renderBuffer.resize(renderWidth * renderHeight * 3, 0);
}

Renderer::~Renderer() {
}

bool Renderer::SetPbrtScene(minipbrt::Scene* scene) {
    if (!scene) {
        std::cerr << "Error: Cannot set null PBRT scene" << std::endl;
        return false;
    }
    
    try {
        this->pbrtScene = scene;
        this->scene = std::make_unique<Scene>(
            PbrtConverter::ConvertScene(scene)
        );
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error converting PBRT scene: " << e.what() << std::endl;
        return false;
    }
}

void Renderer::BeginRender() {
    std::cout << "Starting render ..." << std::endl;
    
    // Reload scene
    if (!LoadScene(sceneFilename)) {
        std::cerr << "Failed to reload scene" << std::endl;
        return;
    }
    
    threadPool = std::make_unique<RenderThreadPool>(scene.get(), NTHREADS, renderWidth, renderHeight);
    threadPool->Stop();
    threadPool->Reset();
    std::fill(renderBuffer.begin(), renderBuffer.end(), 0);
    threadPool->Start([this](int u, int v) { RenderPixel(u, v); });
}

void Renderer::StopRender() {
}

float Renderer::TraceShadowRay(const glm::vec3& o, const glm::vec3& d, const glm::vec3& n, float maxDist) const {
    HitInfo hit;
    Ray shadowRay = Ray(o + n * SHADOW_EPS, d); 
    if (IntersectRayScene(shadowRay, hit)) {
        if (hit.t < maxDist) {
            return 0.0f;
        }
    }
    return 1.0f;
}

bool Renderer::IntersectRayScene(const Ray& ray, HitInfo& hit) const {
    bool hitAny = false;
	float closest = FLT_MAX;
    for(Shape* shape : scene->shapes) {
        Ray rObj = ray.Transform(shape->GetInverseTransform());
        HitInfo tmpHit;
        if(shape->IntersectRay(rObj, tmpHit)) {
			float tWorld = glm::dot(tmpHit.p - ray.o, ray.d); 
            if(tWorld < closest) {
				closest = tWorld;
                tmpHit.t = tWorld;
                hit = tmpHit;
                hitAny = true;
            }
        }
    }
    return hitAny;
}

glm::vec3 Renderer::TracePath(const Ray& ray, Sampler& sampler, int depth) {
    glm::vec3 color(0.0f);

    // Cutoff
    if (depth > MAX_BOUNCES) {
        return color;
    }

    // Russian Roulette
    float pSurvive = 0.95f;
    if (depth > 3){
        float x = sampler.Sample1D();
        if (x > pSurvive) {
            return color;
        }
    }

    // Trace ray
    HitInfo hit;
    if (!IntersectRayScene(ray, hit)) {
        return glm::vec3(0.0f);  // TODO: Environment mapping
    }

    glm::vec3 directLight(0.0f);
    glm::vec3 indirectLight(0.0f);
    int beta = 2; // Power heuristic

    // ===================
    // === 1. Emission ===
    // ===================

    if (hit.areaLightId != minipbrt::kInvalidIndex) {
        AreaLight* areaLight = dynamic_cast<AreaLight*>(scene->lights[hit.areaLightId]);
        // TODO: Might run into indexing issues later with ideal/area light mixup
        if(!areaLight) throw std::runtime_error("Light at areaLightId is not an AreaLight");
        return areaLight->Illuminated(hit, *this, *scene->shapes[hit.areaLightId]);
    }

    // ================================
    // === 2. Random light sampling ===
    // ================================

    // Choose an area or point light at random
    int nLights = static_cast<int>(scene->lights.size());
    float pLight = 1.0f / nLights;
    int lightIdx = sampler.SampleInt(0, nLights - 1);
    Light* light = scene->lights[lightIdx];
    LightSample randomLightSample;

    // TODO: Not huge fan of polymorphism here
    if(IdealLight* idealLight = dynamic_cast<IdealLight*>(light)){
        randomLightSample = idealLight->Sample(hit, *this);
    } else if(AreaLight* areaLight = dynamic_cast<AreaLight*>(light)){
        randomLightSample = areaLight->Sample(hit, *this);
    }

    // =====================================
    // === 3. NEE: Direct light sampling ===
    // =====================================

    Material* mat = scene->materials[hit.materialId];
    if(!mat) throw std::runtime_error("Material at hit.materialId is null");

    // If unoccluded, evaluate BxDF and accumulate direct lighting
    // TODO: Update Illuminated() to support this directly via polymorphism
    glm::vec3 toLight = randomLightSample.position - hit.p;
    if(TraceShadowRay(hit.p, toLight, hit.n, glm::length(toLight)) != 0.0f ){
        float directLightPdf = randomLightSample.pdf * pLight;
        float directLightPower = glm::pow(directLightPdf, beta);

        Shading::BxDFSample bxdfDirectLightSample = Shading::SampleMaterial(hit, mat, glm::normalize(toLight), sampler);
        float bxdfDirectLightPdf = glm::pow(bxdfDirectLightSample.pdf, beta);

        float misWeightDirect = (directLightPower / (directLightPower + bxdfDirectLightPdf));
        directLight += misWeightDirect * randomLightSample.radiance * bxdfDirectLightSample.color / directLightPdf;
    }
    
    // ============================
    // === 4. Indirect lighting ===
    // ============================


    // Sample BxDF to get new path direction
    Shading::BxDFSample bxdfPathSample = Shading::SampleMaterial(hit, mat, ray.d, sampler);
    float bxdfIndirectLightPdf = glm::pow(bxdfPathSample.pdf, beta);

    // Sample all lights to find pdf for  that direction
    float indirectLightPdf = 0.0f;
    for(Light* light : scene->lights){
        if(IdealLight* idealLight = dynamic_cast<IdealLight*>(light)){
            indirectLightPdf += pLight * idealLight->Sample(hit, *this).pdf;
        } else if(AreaLight* areaLight = dynamic_cast<AreaLight*>(light)){
            indirectLightPdf += pLight * areaLight->Sample(hit, *this).pdf;
        }
    }

    float indirectLightPower = glm::pow(indirectLightPdf, beta);
    float misWeightIndirect = bxdfIndirectLightPdf / (indirectLightPower + bxdfIndirectLightPdf);

    // Pathtrace
    depth++;
    Ray bounceRay(hit.p, bxdfPathSample.direction);
    indirectLight += misWeightIndirect * TracePath(bounceRay, sampler, depth) * bxdfPathSample.color / bxdfPathSample.pdf;
    indirectLight /= pSurvive; // RR compensation
    
    // Add contributions
    color += directLight + indirectLight;
    return color;
}

void Renderer::RenderPixel(int u, int v) {
    std::vector<uint8_t>& buffer = GetRenderBuffer();

    Sampler sampler(u * renderWidth + v);
    glm::vec3 color(0.0f);
    int depth = 0;

    // Multi-sampling
    for(int i = 0; i < SPP; i++){
        // Generate halton jittered pixel coordinates
        glm::vec2 jitter = sampler.SampleHalton2D(2, 3, i);

        // Send primary rays to pathtrace
        Ray camRay = scene->camera->GenerateRay(u + jitter.x, v + jitter.y, renderWidth, renderHeight);
        color += TracePath(camRay, sampler, depth);
    }

    // Average
    color /= float(SPP);
    
    // Write to buffer
    int index = (v * renderWidth + u) * 3;
    buffer[index + 0] = static_cast<uint8_t>(glm::clamp(color.r, 0.0f, 1.0f) * 255);
    buffer[index + 1] = static_cast<uint8_t>(glm::clamp(color.g, 0.0f, 1.0f) * 255);
    buffer[index + 2] = static_cast<uint8_t>(glm::clamp(color.b, 0.0f, 1.0f) * 255);
}

bool Renderer::LoadScene(const std::string& filename) {
	sceneFilename = filename;
	PbrtLoader pbrtLoader;
	if(!pbrtLoader.LoadScene(filename)) {
		std::cerr << "Error loading PBRT scene from file: " << filename << std::endl;
		return false;
	}
	return SetPbrtScene(pbrtLoader.GetScene());
}
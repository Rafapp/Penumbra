#include "renderer.h"

#include "pbrtloader.h"
#include "shading.h"

Renderer::Renderer() {
    sampler = Sampler(0);
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

float Renderer::TraceShadowRay(const Ray& ray, float maxDist) {
    HitInfo hit;
    if (IntersectRayScene(ray, hit)) {
        if (hit.t < maxDist) {
            return 0.0f;
        }
    }
    return 1.0f;
}

bool Renderer::IntersectRayScene(const Ray& ray, HitInfo& hit) {
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

glm::vec3 Renderer::TracePath(const Ray& ray, Sampler& sampler, int& depth) {
    glm::vec3 color(0.0f);
    // TODO: Russian Roulette
    if (depth > BOUNCES) return color;

//     HitInfo hit;
//     if (!IntersectRayScene(ray, hit) && depth == 0) return color; // TODO: Environment color

//     glm::vec3 directLight(0.0f);
//     glm::vec3 indirectLight(0.0f);

//     // === 1. Emission ===
//     if (hit.areaLightId != minipbrt::kInvalidIndex) {
//         AreaLight* areaLight = scene->areaLights[hit.areaLightId];
//         return areaLight->Illuminated(hit, *this, *scene->shapes[hit.areaLightId]);
//     }

//     // === 2. Direct Lighting ===
//     // Choose an area light
//     int nLights = static_cast<int>(scene->lights.size());
//     float pLight = 1.0f / nLights;
//     int lightIdx = sampler.SampleInt(0, nLights - 1);
//     AreaLight* areaLight = scene->areaLights[lightIdx];

//     // Get a BXDF sample
//     Material* mat = scene->materials[hit.materialId];
//     Shading::BxDFSample sample = Shading::SampleMaterial(hit, mat, -ray.d, sampler);

//     // Generate bounce ray using sampled direction
//     Ray bounceRay = Ray(hit.p, sample.direction);
//     if(!IntersectRayScene(bounceRay, hit)) return color;

//     // If not in shadow, sample lighting
//     glm::vec3 shadowFactor = areaLight->Illuminated(hit, *this, *scene->shapes[hit.areaLightId]);
//     if(shadowFactor != glm::vec3(0.0f)) {
//         Lights::LightSample lightSample = Lights::SampleAreaLight(areaLight, hit, *this);
//         directLight += ; 
//     }
//     glm::vec3 direct = L * Shading::ShadeMaterial(hit, mat, scene->lights);
//     glm::vec3 indirect = TracePath(bounceRay, sampler, depth) * (sample.color / sample.pdf);
//     color += direct + indirect;
        
//     depth++;

//     Lights::LightSample lightSample = Lights::SampleAreaLight(areaLight, hit);
//     // PI * Surface Area * Emitted Radiance(I * shadow ray)
//     indirectLight += lightSample.radiance * M_PI * areaLight->GetSurfaceArea();


//    return color; 
}

void Renderer::RenderPixel(int u, int v) {
    std::vector<uint8_t>& buffer = GetRenderBuffer();

    // Send primary ray to pathtrace
    int depth = 0;
    Ray camRay = scene->camera->GenerateRay(u, v, renderWidth, renderHeight);
    glm::vec3 color = TracePath(camRay, sampler, depth);

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
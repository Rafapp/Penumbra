#include "renderer.h"

#include "pbrtloader.h"
#include "shading.h"

Renderer::Renderer() {
    scene = std::make_unique<Scene>();
    renderBuffer.resize(renderWidth * renderHeight * 3, 0);
}

Renderer::~Renderer() {
    // scene.reset();
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
    if(threadPool) threadPool->Stop();
    std::cout << "Starting render ..." << std::endl;
    
    // Reload scene
    if (!LoadScene(sceneFilename)) {
        std::cerr << "Failed to reload scene" << std::endl;
        return;
    }
    renderWidth = gui->GetRenderWidth();
    renderHeight = gui->GetRenderHeight();
    spp = gui->GetSPP();
    renderBuffer.resize(renderWidth * renderHeight * 3, 0);
    std::fill(renderBuffer.begin(), renderBuffer.end(), 0);

    threadPool = std::make_unique<RenderThreadPool>(scene.get(), NTHREADS, renderWidth, renderHeight);
    threadPool->Start([this](int u, int v) { RenderPixel(u, v); });
}

void Renderer::StopRender() {
}

bool Renderer::Occluded(const glm::vec3& p, const glm::vec3& wi, const glm::vec3& n, float maxDist) const {
    HitInfo hit;
    Ray shadowRay = Ray(p + n * OCCLUDED_EPS, wi);
    for(Shape* shape : scene->shapes) {
        if(shape->IsAreaLight()) continue;
        Ray rObj = shadowRay.Transform(shape->GetInverseTransform());
        HitInfo hit;
        if(shape->IntersectRay(rObj, hit)) {
            if (hit.front && hit.t < maxDist) {
                return true;
            }
        }
    }
    return false;
}

bool Renderer::TraceRay(const Ray& ray, HitInfo& hit) const {
    bool hitAny = false;
	float closest = FLT_MAX;
    for(Shape* shape : scene->shapes) {
        Ray rObj = ray.Transform(shape->GetInverseTransform());
        HitInfo tmpHit;
        if(shape->IntersectRay(rObj, tmpHit)) {
            if(tmpHit.t < closest) {
				closest = tmpHit.t;
                tmpHit.shape = shape;
                tmpHit.material = shape->material;
                tmpHit.areaLight = shape->areaLight;
                hit = tmpHit;
                hitAny = true;
            }
        }
    }
    return hitAny;
}

glm::vec3 Renderer::TracePath(const Ray& ray, Sampler& sampler, int depth, glm::vec3 throughput) {
    glm::vec3 color(0.0f);

    // Cutoff
    if(INF_BOUNCES == false) {
        if (depth > MAX_BOUNCES) {
            return color;
        }
    }

    // Russian Roulette
    float pSurvive = 1.0f;
    if (depth > 1){
        float maxEnergy = glm::max(glm::max(throughput.r, throughput.g), throughput.b);
        pSurvive = glm::max(0.05f, maxEnergy);
        float x = sampler.Sample1D();
        if (x > pSurvive) {
            return color;
        }
        throughput /= pSurvive;
    }

    // Trace ray
    HitInfo hit;
    if (!TraceRay(ray, hit)) {
        return glm::vec3(0.0f);  // TODO: Environment mapping
    }

    glm::vec3 directLight(0.0f);
    glm::vec3 indirectLight(0.0f);
    int beta = 2; // Power heuristic

    // ===================
    // === 1. Emission ===
    // ===================

    if (hit.areaLight != nullptr) {
        AreaLight* areaLight = dynamic_cast<AreaLight*>(hit.areaLight);
        return areaLight->GetRadiance(hit,*areaLight->shape);
    }

    // ================================
    // === 2. Random light sampling ===
    // ================================

    // Choose an area or point light at random
    int nLights = static_cast<int>(scene->lights.size());
    float pLight = 1.0f / nLights;
    int lightIdx = sampler.SampleInt(0, nLights - 1);
    Light* light = scene->lights[lightIdx];
    LightSample randomIdealLightSample;
    LightSample randomAreaLightSample;

    // TODO: Not huge fan of polymorphism here
    if(IdealLight* idealLight = dynamic_cast<IdealLight*>(light)){
        if(!light) throw std::runtime_error("Randomly selected ideal light is null");
        randomIdealLightSample = idealLight->Sample(hit, sampler);
    } else if(AreaLight* areaLight = dynamic_cast<AreaLight*>(light)){
        randomAreaLightSample = areaLight->Sample(hit, sampler, *areaLight->shape);
        if(!areaLight) throw std::runtime_error("Randomly selected area light is null");
    }

    // =================================
    // === 3. NEE: With random light ===
    // =================================

    Material* mat = hit.material;
    if(!mat) throw std::runtime_error("Material at hit.material is null");

    // 1. Ideal light
    // if(randomIdealLightSample.pdf > 0.0f){
    //     glm::vec3 toLight = randomIdealLightSample.position - hit.p;
    //     if(Occluded(hit.p, toLight, hit.n, glm::length(toLight)) != 0.0f ){
    //         float directLightPdf = randomIdealLightSample.pdf * pLight;
    //         float directLightPower = glm::pow(directLightPdf, beta);

    //         Shading::BxDFSample bxdfDirectLightSample = Shading::SampleMaterial(hit, mat, glm::normalize(toLight), sampler);
    //         float bxdfDirectLightPower = glm::pow(bxdfDirectLightSample.pdf, beta);

    //         float misWeightDirect = (directLightPower / (directLightPower + bxdfDirectLightPower));
    //         directLight += misWeightDirect * randomIdealLightSample.radiance * bxdfDirectLightSample.color / directLightPdf;
    //     }
    // }

    // 2. Area light (assuming extended point light lambertian emitter for now)
    if (randomAreaLightSample.pdf > 0.0f) {
        glm::vec3 toLight = randomAreaLightSample.p - hit.p;
        float d = glm::length(toLight);
        glm::vec3 wi = toLight / d;
        if(!Occluded(hit.p, wi, hit.n, d)){
            float pdf = randomAreaLightSample.pdf * pLight;
            if(pdf > 0.0f){
                float cosHit = glm::max(0.0f, glm::dot(hit.n, wi)) / (d * d);
                directLight += throughput * cosHit * randomAreaLightSample.L * Shading::ShadeMaterial(hit, wi, mat) / (pdf);
            }
        }
    }
    
    // ============================
    // === 4. Indirect lighting ===
    // ============================

    Shading::BxDFSample bxdfPathSample = Shading::SampleMaterial(hit, mat, ray.d, sampler);
    float pdf = bxdfPathSample.pdf;
    if (pdf > 0.0f) {
        depth++;
        Ray bounceRay(hit.p, bxdfPathSample.direction);
        float cosPath = glm::max(0.0f, glm::dot(hit.n, bxdfPathSample.direction));
        glm::vec3 newThroughput = throughput * bxdfPathSample.color * cosPath / pdf;
        indirectLight += TracePath(bounceRay, sampler, depth, newThroughput);
    }
    
    // Add contributions
    color += directLight + indirectLight;
    return color;
}

void Renderer::RenderPixel(int u, int v) {
    std::vector<uint8_t>& buffer = GetRenderBuffer();

    Sampler sampler(u * renderWidth + v);
    glm::vec3 color(0.0f);
    int depth = 0;

    for(int i = 0; i < spp; i++){
        glm::vec2 jitter = sampler.SampleHalton2D(2, 3, i);
        Ray camRay = scene->camera->GenerateRay(u + jitter.x, v + jitter.y, renderWidth, renderHeight);
        glm::vec3 sample = TracePath(camRay, sampler, depth);
        color += sample;
    }

    // Average over samples
    color /= float(spp);
    
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
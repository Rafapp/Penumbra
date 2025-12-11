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

    auto rs = gui->GetRenderSettings();
    renderWidth = rs.width;
    renderHeight = rs.height;
    spp = rs.spp;
    indirectLighting = rs.indirect;
    misEnabled = rs.mis;
	gammaCorrect = rs.gammaCorrect;
	tonemap = rs.tonemap;
    exposureBias = rs.exposureBias;

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

    // Russian Roulette
    float pSurvive = 1.0f;
    if (depth > 1){
        float maxEnergy = glm::max(glm::max(throughput.r, throughput.g), throughput.b);
        pSurvive = glm::min(0.99f, glm::max(0.1f, maxEnergy));
        float x = sampler.Sample1D();
        if (x > pSurvive) {
            return glm::vec3(0.0f);
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
        return throughput * areaLight->GetRadiance(hit, *hit.shape);
    }

    Material* mat = hit.material;
	if (!mat) return glm::vec3(0.0f);

    // ==============================
    // === 2. Sample random light ===
    // ==============================

    LightSample randomIdealLightSample;
    randomIdealLightSample.pdf = 0.0f;
    LightSample randomAreaLightSample;
    randomAreaLightSample.pdf = 0.0f;

    int nLights = static_cast<int>(scene->lights.size());
    int lightIdx = sampler.SampleInt(0, nLights - 1);
    float pLight = 1.0f / nLights;
    Light* light = scene->lights[lightIdx];

    // TODO: Not huge fan of polymorphism here
    IdealLight* idealLight = dynamic_cast<IdealLight*>(light);
    AreaLight* areaLight = dynamic_cast<AreaLight*>(light);

    if(idealLight){
        if(!idealLight) throw std::runtime_error("Renderer::TracePath: Randomly selected ideal light is null");
        randomIdealLightSample = idealLight->Sample(hit, sampler);
    } else if(areaLight){
        if(!areaLight) throw std::runtime_error("Renderer::TracePath: Randomly selected area light is null");
        randomAreaLightSample = areaLight->Sample(hit, *this, sampler, *areaLight->shape);
    }

    // ================================
    // === 3. Next event estimation ===
    // ================================

	// 1. Ideal light
	if(randomIdealLightSample.pdf > 0.0f){
		glm::vec3 tl = randomIdealLightSample.p - hit.p;
		float dl = glm::length(randomIdealLightSample.p - hit.p);
		glm::vec3 wi = tl / dl;
		glm::vec3 wo = -ray.d;
		if(!Occluded(hit.p, wi, hit.n, dl)){
			float lightPdf = randomIdealLightSample.pdf * pLight;
			if(lightPdf > 0.0f){
				glm::vec3 matColor = Shading::ShadeMaterial(hit, wi, wo, mat);
                float mis = 1.0f;
                if (misEnabled) {
                    float bxdfPdf = Shading::PdfMaterial(hit, wi, wo, mat, sampler);
                    float bxdfPower = glm::pow(bxdfPdf, beta);
                    float lightPower = glm::pow(lightPdf, beta);
                    mis = lightPower / (lightPower + bxdfPower);
                }
                directLight += throughput * mis * randomIdealLightSample.L * matColor / lightPdf;
			}
		}
	}

	// 2. Area light (assuming scaled point light lambertian emitter for now)
	if(randomAreaLightSample.pdf > 0.0f){
		glm::vec3 tl = randomAreaLightSample.p - hit.p;
		float dl = glm::length(randomAreaLightSample.p - hit.p);
		glm::vec3 wi = tl / dl;
		glm::vec3 wo = -ray.d;
		if(!Occluded(hit.p, wi, hit.n, dl)){
			float lightPdf = randomAreaLightSample.pdf * pLight;
			if(lightPdf > 0.0f){
				glm::vec3 matColor = Shading::ShadeMaterial(hit, wi, wo, mat);
                float mis = 1.0f;
                if(misEnabled) {
					float lightPower = glm::pow(lightPdf, beta);
					float bxdfPdf = Shading::PdfMaterial(hit, wi, wo, mat, sampler);
					float bxdfPower = glm::pow(bxdfPdf, beta);
					mis = lightPower / (lightPower + bxdfPower);
				}
				directLight += throughput * mis * randomAreaLightSample.L * matColor / lightPdf;
			}
		}
	}
    
    // ============================
    // === 4. Indirect lighting ===
    // ============================

    if(indirectLighting) {
        glm::vec3 tl = randomAreaLightSample.p - hit.p;
        float dl = glm::length(randomAreaLightSample.p - hit.p);
        glm::vec3 wi = tl / dl;
        glm::vec3 wo = -ray.d;

        // Sample BxDF for new direction
        Shading::BxDFSample matSample = Shading::SampleMaterial(hit, wi, wo, mat, sampler);
        float bxdfPdf = matSample.pdf;

        // Find light's PDF at sampled direction
        float lightPdf = 0.0f;
        if(idealLight){
            lightPdf = idealLight->Pdf(hit, matSample.wo) * pLight;
        } else if (areaLight){
            lightPdf = areaLight->Pdf(hit, *this, matSample.wo) *  pLight;
        }

        depth++;
        float mis = 1.0f;
        if (misEnabled) {
            float lightPower = glm::pow(lightPdf, beta);
            float bxdfPower = glm::pow(bxdfPdf, beta);
            mis = bxdfPower / (lightPower + bxdfPower);
        }
        float cos = glm::max(0.0f, glm::dot(hit.n, matSample.wo));
        glm::vec3 newThroughput = mis * throughput * cos * matSample.color / bxdfPdf;
        Ray bounceRay(hit.p + OCCLUDED_EPS * hit.n, matSample.wo);
        glm::vec3 Li = TracePath(bounceRay, sampler, depth, newThroughput);
        indirectLight += Li;
    }

    // Add contributions
    if(!indirectLighting) return directLight;
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

    // Gamma correct and tonemap
    if(tonemap) Color::UnchartedTonemapFilmic(color, exposureBias);
    if(gammaCorrect) Color::GammaCorrect(color);
    
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
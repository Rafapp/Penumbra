#include "tinybvh_glm_config.h"
#include "tiny_bvh.h"

#include "renderer.h"
#include "scene.h"
#include "shading.h"
#include "pbrtloader.h"

Renderer::Renderer() {
    scene = std::make_unique<Scene>();
    renderBuffer.resize(renderWidth * renderHeight * 3, 0);
    // TODO: Move to pbrt file or GUI
	// const std::string envMapFile = "./resources/images/snooker-room-4k.exr";
    const std::string envMapFile = "./resources/images/grocery-store-4k.exr";
    envMap = EnvironmentMap(envMapFile);
    if(!envMap.Load()) {
        std::cerr << "Failed to load environment map: " << envMapFile << std::endl;
    }
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

void Renderer::PrintStats() {
    // ANSI colors (most terminals). If your console doesn’t support it, set USE_COLOR=false.
    constexpr bool USE_COLOR = true;

    auto c = [&](const char* code) -> const char* { return USE_COLOR ? code : ""; };

    constexpr const char* RST  = "\x1b[0m";
    constexpr const char* BLD  = "\x1b[1m";
    constexpr const char* DIM  = "\x1b[2m";

    constexpr const char* LINE = "\x1b[38;5;239m";  // dark gray
    constexpr const char* HDR  = "\x1b[38;5;81m";   // cyan
    constexpr const char* SEC  = "\x1b[38;5;215m";  // warm orange
    constexpr const char* KEY  = "\x1b[38;5;250m";  // light gray
    constexpr const char* NUM  = "\x1b[38;5;111m";  // blue-ish
    constexpr const char* ON   = "\x1b[38;5;120m";  // green
    constexpr const char* OFF  = "\x1b[38;5;203m";  // red

    auto yn = [&](bool v) -> std::string {
    return std::string(c(v ? ON : OFF)) + (v ? "ENABLED" : "DISABLED") + c(RST);
};

// Layout constants
constexpr int KW    = 22; // key width for alignment
constexpr int INNER = 54; // interior width between the vertical bars

const std::string title = "STARTING RENDER";

// Center title (visible chars only; color codes are not counted in title.size()).
const int leftPad  = (INNER - static_cast<int>(title.size())) / 2;
const int rightPad = INNER - static_cast<int>(title.size()) - leftPad;

std::cout
    << c(LINE) << "┏" << std::string(INNER, '-') << "┓" << c(RST) << "\n"
    << c(LINE) << "┃"
    << std::string(leftPad, ' ')
    << c(BLD) << c(HDR) << title << c(RST)
    << c(LINE) << std::string(rightPad, ' ')
    << "┃" << c(RST) << "\n"
    << c(LINE) << "┗" << std::string(INNER, '-') << "┛" << c(RST) << "\n";

    // === Render Parameters ===
    std::cout << c(BLD) << c(SEC) << "◆ Render Parameters" << c(RST) << "\n";
    std::cout << "  " << c(KEY) << std::left << std::setw(KW) << "Resolution"
              << c(RST) << c(DIM) << " : " << c(RST)
              << c(NUM) << renderWidth << c(RST) << c(DIM) << " x " << c(RST)
              << c(NUM) << renderHeight << c(RST) << "\n";

    std::cout << "  " << c(KEY) << std::left << std::setw(KW) << "Samples per pixel"
              << c(RST) << c(DIM) << " : " << c(RST)
              << c(NUM) << spp << c(RST) << "\n";

    std::cout << "  " << c(KEY) << std::left << std::setw(KW) << "Indirect lighting"
              << c(RST) << c(DIM) << " : " << c(RST)
              << yn(indirectLighting) << "\n";

    std::cout << "  " << c(KEY) << std::left << std::setw(KW) << "MIS"
              << c(RST) << c(DIM) << " : " << c(RST)
              << yn(misEnabled) << "\n";

    std::cout << c(LINE) << "  ────────────────────────────────────────────────────" << c(RST) << "\n";

    // === Scene Statistics ===
    std::cout << c(BLD) << c(SEC) << "◆ Scene Statistics" << c(RST) << "\n";
    std::cout << "  " << c(KEY) << std::left << std::setw(KW) << "Number of shapes"
              << c(RST) << c(DIM) << " : " << c(RST)
              << c(NUM) << scene->shapes.size() << c(RST) << "\n";

    std::cout << "  " << c(KEY) << std::left << std::setw(KW) << "Number of lights"
              << c(RST) << c(DIM) << " : " << c(RST)
              << c(NUM) << scene->lights.size() << c(RST) << "\n";

    std::cout << "  " << c(KEY) << std::left << std::setw(KW) << "Number of materials"
              << c(RST) << c(DIM) << " : " << c(RST)
              << c(NUM) << scene->materials.size() << c(RST) << "\n";

    std::cout << c(LINE) << "  ────────────────────────────────────────────────────" << c(RST) << "\n";
}

bool Renderer::SaveImage() {
    if (imgOutPath[0] == '\0' || imgName[0] == '\0') {
        std::cerr << "SaveImage error: output path or filename not set" << std::endl;
        return false;
    }
    
    if (renderBuffer.empty()) {
        std::cerr << "SaveImage error: render buffer is empty" << std::endl;
        return false;
    }
    
    bool stereo = false;
    bool leftEye = false;
    
    auto fullPath = (std::filesystem::path(imgOutPath) / imgName).string();
    
    OIIO::ImageSpec spec(renderWidth, renderHeight, 3, OIIO::TypeDesc::UINT8);
    auto out = OIIO::ImageOutput::create(fullPath);
    if (!out) {
        std::cerr << "SaveImage error (create): " << OIIO::geterror() << std::endl;
        return false;
    }
    if (!out->open(fullPath, spec)) {
        std::cerr << "SaveImage error (open): " << out->geterror() << std::endl;
        return false;
    }
    
    const uint8_t* src = renderBuffer.data();
    bool writeSuccess = true;
    
    if (!stereo) {
        writeSuccess = out->write_image(OIIO::TypeDesc::UINT8, src);
    } else {
        std::vector<uint8_t> stereoBuffer(renderWidth * renderHeight * 3);
        for (int i = 0; i < renderWidth * renderHeight; ++i) {
            uint8_t r = src[3 * i + 0];
            uint8_t g = src[3 * i + 1];
            uint8_t b = src[3 * i + 2];
            
            if (leftEye) {
                stereoBuffer[3 * i + 0] = r;
                stereoBuffer[3 * i + 1] = 0;
                stereoBuffer[3 * i + 2] = 0;
            }
            else {
                stereoBuffer[3 * i + 0] = 0;
                stereoBuffer[3 * i + 1] = g;
                stereoBuffer[3 * i + 2] = b;
            }
        }
        writeSuccess = out->write_image(OIIO::TypeDesc::UINT8, stereoBuffer.data());
    }
    
    if (!writeSuccess) {
        std::cerr << "SaveImage error (write): " << out->geterror() << std::endl;
        out->close();
        return false;
    }
    
    out->close();
    std::cout << "Saved: " << fullPath << std::endl;
    return true;
}

// Render all animation frames (.pbrt scenes) in a folder
void Renderer::RenderAnimation(){
    auto rs = gui->GetRenderSettings();
    strncpy(animPath, rs.animPath, sizeof(animPath) - 1);
    animPath[sizeof(animPath) - 1] = '\0';
    strncpy(animSavePath, rs.saveAnimPath, sizeof(animSavePath) - 1);
    animSavePath[sizeof(animSavePath) - 1] = '\0';

    std::vector<std::string> sceneFiles; 
    for (const auto& entry : std::filesystem::recursive_directory_iterator(animPath)) {
        if (entry.is_regular_file() && entry.path().extension() == ".pbrt") {
            sceneFiles.push_back(entry.path().string());
        }
    }

    int ct = sceneFiles.size();
    if(ct == 0){
        std::cout << "No PBRT scenes found in path: " << animPath << std::endl;
        return;
    }

    std::cout << "Found " << ct << " PBRT scenes in path: " << animPath << std::endl;
    std::cout << "Rendering animation ..." << std::endl;

    // Sort frame files numerically
    std::sort(sceneFiles.begin(), sceneFiles.end(), [](const std::string& a, const std::string& b) {
        int numA = std::stoi(std::filesystem::path(a).stem().string());
        int numB = std::stoi(std::filesystem::path(b).stem().string());
        return numA < numB;
    });

    // Render all frames
    int i = 1;
    for (const auto& sceneFile : sceneFiles) {
        std::cout << "Rendering frame: " << i << " out of: " << ct << std::endl;
        strncpy(scenePath, sceneFile.c_str(), sizeof(scenePath) - 1);
        scenePath[sizeof(scenePath) - 1] = '\0';
        BeginRender();
        threadPool->frameFinished = false;
        while(!threadPool->frameFinished){
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        std::cout << "Finished rendering frame: " << i << std::endl;

        // Update filename and path
        std::string fName = std::to_string(i) + ".png";
        strncpy(imgName, fName.c_str(), sizeof(imgName) - 1);
        imgName[sizeof(imgName) - 1] = '\0';
        strncpy(imgOutPath, animSavePath, sizeof(imgOutPath) - 1);
        imgOutPath[sizeof(imgOutPath) - 1] = '\0';
        std::cout << "imgName is: " << imgName << std::endl;
        std::cout << "imgOutPath is: " << imgOutPath << std::endl;

        // Save frame
        std::cout << "Saving image ..." << std::endl;
        if(SaveImage()){
            std::cout << "Saved image " << fName << " successfully." << std::endl;
        } else {
            std::cout << "Failed to save image " << fName << "." << std::endl;
            return;
        }
        std::cout << std::endl;
        i++;
    }
}

void Renderer::BeginRender() {
    if(threadPool) threadPool->Stop();
    std::cout << "Starting render ..." << std::endl;
    auto rs = gui->GetRenderSettings();
    
    // Reload scene
    if (!LoadScene(scenePath)) {
        std::cerr << "Failed to reload scene" << std::endl;
        return;
    }
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
    PrintStats();
    threadPool = std::make_unique<RenderThreadPool>(scene.get(), NTHREADS, renderWidth, renderHeight);
    threadPool->startTime = std::chrono::steady_clock::now();
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
    // TODO: Use TLAS (top layer acceleration structure) for faster traversal
    for (Shape* shape : scene->shapes) {
        Ray rObj = ray.Transform(shape->GetInverseTransform());
        HitInfo tmpHit;
        tmpHit.t = closest;
        if (shape->IntersectRay(rObj, tmpHit)) {
            if (tmpHit.t < closest) {
                closest = tmpHit.t;
                tmpHit.shape = shape;

                // Material lookup by triangle index
                if (auto mesh = dynamic_cast<TriangleMesh*>(shape)) {
                    if (mesh->triMaterialIndices && tmpHit.triangleIndex < mesh->triMaterialIndices->size()) {
                        uint32_t matIdx = (*mesh->triMaterialIndices)[tmpHit.triangleIndex];
                        tmpHit.material = scene->materials[matIdx];
                    }
                } else {
                    tmpHit.material = shape->material;
                }

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

    // Trace ray
    HitInfo hit;
    hit.t = FLT_MAX;
    if (!TraceRay(ray, hit)) {
        return envMap.SampleColor(ray);
    }

    // Russian Roulette
    if(depth > 32) return glm::vec3(0.0f); // Hard cutoff
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
	if (!mat) glm::vec3(0.0f);

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
        if(!idealLight) throw std::runtime_error("Renderer::TracePath: Randomly selected ideal light that is null");
        randomIdealLightSample = idealLight->Sample(hit, sampler);
    } else if(areaLight){
        if(!areaLight) throw std::runtime_error("Renderer::TracePath: Randomly selected area light that is null");
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
		glm::vec3 n = hit.front ? hit.n : -hit.n;
		if(!Occluded(hit.p, wi, n, dl)){
			float lightPdf = randomIdealLightSample.pdf * pLight;
			if(lightPdf > 0.0f){
                // TODO: Proper delta check
				float bxdfPdf = Shading::PdfMaterial(hit, wi, wo, mat, sampler);
                float mis = 1.0f;
                if (bxdfPdf > 0.0f) {
                    if (misEnabled) {
                        float bxdfPower = glm::pow(bxdfPdf, beta);
                        float lightPower = glm::pow(lightPdf, beta);
                        mis = lightPower / (lightPower + bxdfPower);
                    }
                    // Evaluate rendering equation
                    glm::vec3 fbrdf = Shading::ShadeMaterial(hit, wi, wo, mat);
                    directLight += mis * throughput * randomIdealLightSample.L * fbrdf * randomIdealLightSample.weight;
                }
			}
		}
	}

	// 2. Area light (assuming scaled point light lambertian emitter for now)
	if(randomAreaLightSample.pdf > 0.0f){
		glm::vec3 tl = randomAreaLightSample.p - hit.p;
		float dl = glm::length(randomAreaLightSample.p - hit.p);
		glm::vec3 wi = tl / dl;
		glm::vec3 wo = -ray.d;
		glm::vec3 n = hit.front ? hit.n : -hit.n;
		if(!Occluded(hit.p, wi, n, dl)){
			float lightPdf = randomAreaLightSample.pdf * pLight;
            if (lightPdf > 0.0f) {
                // TODO: Proper delta check
				float bxdfPdf = Shading::PdfMaterial(hit, wi, wo, mat, sampler);
                float mis = 1.0f;
                if (bxdfPdf > 0.0f) {
                    if (misEnabled) {
                        float lightPower = glm::pow(lightPdf, beta);
                        float bxdfPower = glm::pow(bxdfPdf, beta);
                        mis = lightPower / (lightPower + bxdfPower);
                    }
                    // Evaluate rendering equation
                    glm::vec3 fbrdf = Shading::ShadeMaterial(hit, wi, wo, mat);
                    directLight += mis * throughput * randomAreaLightSample.L * fbrdf * randomAreaLightSample.weight;
                }
            }
		}
	}
    
    // ===========================================
    // === 4. Indirect lighting (Path Tracing) ===
    // ===========================================

    if(indirectLighting) {

        // Sample BxDF for new direction
        glm::vec3 rd = -ray.d;
        Shading::BxDFSample matSample = Shading::SampleMaterial(hit, rd, mat, sampler);
        float bxdfPdf = matSample.pdf;
        if (bxdfPdf <= 0.0f) return glm::vec3(0.0f); // Invalid path, terminate 

        // Find light's PDF at sampled direction
        float lightPdf = 0.0f;
        if(idealLight){
            lightPdf = idealLight->Pdf(hit, matSample.wo) * pLight;
        } else if (areaLight){
            lightPdf = areaLight->Pdf(hit, *this, matSample.wo) * pLight;
        }

        depth++;
        float mis = 1.0f;
        if (misEnabled) {
            if (lightPdf == 0.0f) mis = 1.0f; // Ignore delta events
            else {
                float lightPower = glm::pow(lightPdf, beta);
                float bxdfPower = glm::pow(bxdfPdf, beta);
                mis = bxdfPower / (lightPower + bxdfPower);
            }
        }

		// Evaluate rendering equation recursively
		glm::vec3 f = matSample.isDelta ? glm::vec3(1.0f) : Shading::ShadeMaterial(hit, -ray.d, matSample.wo, mat);
        glm::vec3 newThroughput = mis * throughput * f * matSample.weight;
		glm::vec3 n = hit.front ? hit.n : -hit.n;
        glm::vec3 offN = (glm::dot(matSample.wo, n) > 0.0f) ? n : -n;
        Ray bounceRay(hit.p + OCCLUDED_EPS * offN, matSample.wo);
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
    glm::vec3 color(0.0f);

    // Miss ray / Env. map (unjittered)
    Ray envRay = scene->camera->GenerateRay(u, v, renderWidth, renderHeight);
    HitInfo hit;
    hit.t = FLT_MAX;

    bool hitAny = TraceRay(envRay, hit);
    if (!hitAny) {
        color += envMap.SampleColor(envRay);
    } 
    // Pathtrace and super sample
    else {
        Sampler sampler(u * renderWidth + v);
        int depth = 0;
        for(int i = 0; i < spp; i++){
            glm::vec2 jitter = sampler.SampleHalton2D(2, 3, i);
            Ray camRay = scene->camera->GenerateRay(u + jitter.x, v + jitter.y, renderWidth, renderHeight);
            glm::vec3 sample = TracePath(camRay, sampler, depth);
            color += sample;
        }
        // Average over samples
        color /= float(spp);
    }

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
    // TODO: Better scene file pipeline
    strncpy(scenePath, filename.c_str(), sizeof(scenePath) - 1);
    scenePath[sizeof(scenePath) - 1] = '\0';
	PbrtLoader pbrtLoader;
	if(!pbrtLoader.LoadScene(scenePath)) {
		std::cerr << "Error loading PBRT scene from file: " << scenePath << std::endl;
		return false;
	}
	return SetPbrtScene(pbrtLoader.GetScene());
}
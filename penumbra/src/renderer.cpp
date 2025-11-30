#include "renderer.h"

#include <random> // TODO: Remove

#include "shading.h"
#include "pbrtloader.h"


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

bool Renderer::IntersectRayScene(const Ray& ray, HitInfo& hit) {
    bool hitAny = false;
    float closest = FLT_MAX;
    for(Shape* shape : scene->shapes) {
        Ray rObj = ray.Transform(shape->GetInverseTransform());
        HitInfo tmpHit;
        if(shape->IntersectRay(rObj, tmpHit)) {
			float tWorld = glm::dot(tmpHit.p - ray.o, ray.d); 
            if(tmpHit.t < closest) {
                closest = tWorld;
                hit = tmpHit;
                hitAny = true;
            }
        }
    }
    
    return hitAny;
}

void Renderer::RenderPixel(int u, int v) {
    std::vector<uint8_t>& buffer = GetRenderBuffer();
    Ray camRay = scene->camera->GenerateRay(u, v, renderWidth, renderHeight);
    
    HitInfo hit;
    if(IntersectRayScene(camRay, hit)) {
		// Diffuse shading for testing
		Material* mat = scene->materials[hit.materialId];
		glm::vec3 color = Shading::ShadeMaterial(hit, mat, scene->lights);

		int index = (v * renderWidth + u) * 3;
		buffer[index + 0] = static_cast<uint8_t>(glm::clamp(color.r, 0.0f, 1.0f) * 255);
		buffer[index + 1] = static_cast<uint8_t>(glm::clamp(color.g, 0.0f, 1.0f) * 255);
		buffer[index + 2] = static_cast<uint8_t>(glm::clamp(color.b, 0.0f, 1.0f) * 255);
        
    } else {
		// No hit, environment map
    }
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

	// Color24* img = renderImage.GetPixels();
	// float* zimg = renderImage.GetZBuffer();
	// int* sampleCountImg = renderImage.GetSampleCount();

	// Matrix4f camToWorld(
	// 	Vec4f(camera.dir.Cross(camera.up), 0.0f),
	// 	Vec4f(camera.up, 0.0f),
	// 	Vec4f(camera.dir, 0.0f),
	// 	Vec4f(camera.pos, 1.0f)
	// );

	// rng = RNG(u + v * camera.imgWidth);
	// float z = BIGFLOAT;
	// float offsetU = rng.RandomFloat();
	// float offsetV = rng.RandomFloat();

	// haltonGenX.Precompute(2);
	// haltonGenY.Precompute(3);
	// haltonGenDofU.Precompute(5);
	// haltonGenDofV.Precompute(7);
	// haltonGenLightU.Precompute(11);
	// haltonGenLightV.Precompute(13);

	// Color result(0.0f);
	// Color mean(0.0f), mean2(0.0f);

	// int nSamples = 0;

	// // Precomputed t-values for 95% confidence interval with degrees of freedom from 1 to 32
	// constexpr float tValues[128] = {
	// 	12.706f, 4.303f, 3.182f, 2.776f, 2.571f, 2.447f, 2.365f, 2.306f,
	// 	2.262f, 2.228f, 2.201f, 2.179f, 2.160f, 2.145f, 2.131f, 2.120f,
	// 	2.110f, 2.101f, 2.093f, 2.086f, 2.080f, 2.074f, 2.069f, 2.064f,
	// 	2.060f, 2.056f, 2.052f, 2.048f, 2.045f, 2.042f, 2.040f, 2.037f,
	// 	2.035f, 2.032f, 2.030f, 2.028f, 2.026f, 2.024f, 2.023f, 2.021f,
	// 	2.020f, 2.018f, 2.017f, 2.015f, 2.014f, 2.013f, 2.012f, 2.011f,
	// 	2.010f, 2.009f, 2.008f, 2.007f, 2.006f, 2.005f, 2.004f, 2.003f,
	// 	2.002f, 2.002f, 2.001f, 2.000f, 2.000f, 1.999f, 1.998f, 1.998f,
	// 	1.997f, 1.997f, 1.996f, 1.995f, 1.995f, 1.994f, 1.994f, 1.994f,
	// 	1.993f, 1.993f, 1.993f, 1.992f, 1.992f, 1.991f, 1.991f, 1.990f,
	// 	1.990f, 1.990f, 1.989f, 1.989f, 1.988f, 1.988f, 1.988f, 1.987f,
	// 	1.987f, 1.987f, 1.986f, 1.986f, 1.986f, 1.985f, 1.985f, 1.985f,
	// 	1.984f, 1.984f, 1.984f, 1.983f, 1.983f, 1.983f, 1.983f, 1.983f,
	// 	1.982f, 1.982f, 1.982f, 1.982f, 1.981f, 1.981f, 1.981f, 1.981f,
	// 	1.981f, 1.980f, 1.980f, 1.980f, 1.980f, 1.980f, 1.979f, 1.979f,
	// 	1.979f, 1.979f, 1.979f, 1.978f, 1.978f, 1.978f, 1.978f, 1.978f
	// };

	// assert(MAX_SPP <= (sizeof(tValues) / sizeof(tValues[0])));

	// // TODO: Make sampling deterministic in each render
	// for(int i = 0; i < MAX_SPP; i++){
	// 	float uOffset = haltonGenX[i] + offsetU;
	// 	if (uOffset > 1.0f) uOffset -= 1.0f;

	// 	float vOffset = haltonGenY[i] + offsetV;
	// 	if( vOffset > 1.0f) vOffset -= 1.0f;

	// 	float uSample = u + uOffset;
	// 	float vSample = v + vOffset;

	// 	Ray viewRay;
	// 	viewRay.p = Vec3f(camToWorld * Vec4f(SampleDof(rng, i), 0.0f, 1.0f));
	// 	viewRay.dir = (Vec3f(camToWorld * Vec4f(FindViewplanePoints(uSample, vSample), 1.0f)) - viewRay.p).GetNormalized();

	// 	HitInfo hit;
	// 	hit.z = BIGFLOAT;
	// 	if (TraceRay(viewRay, hit)) {
	// 		ShadingData data(scene.lights, scene.environment, this, BOUNCES, rng);
	// 		data.SetHit(viewRay, hit);
	// 		z = hit.z;
	// 		result = hit.node->GetMaterial()->Shade(data);
	// 	} else {
	// 		result = scene.background.Eval(Vec3f((u + 0.5f) / camera.imgWidth, (v + 0.5f) / camera.imgHeight, 0.0f));
	// 	}
	// 	nSamples++;

	// 	// Welford's method for online mean and variance calculation
	// 	Color delta = result - mean;
	// 	mean += delta / float(i + 1);
	// 	Color delta2 = result - mean;
	// 	mean2 += delta * delta2;

	// 	// Exit early if error is below threshold and we covered min samples (Adaptive Sampling)
	// 	if(i > MIN_SPP ){
	// 		Color variance = mean2 / float(i - 1);
	// 		Color stdDev(
	// 			sqrtf(variance.r),
	// 			sqrtf(variance.g),
	// 			sqrtf(variance.b)
	// 		);
	// 		Color error = stdDev * tValues[i] / sqrtf(float(i + 1));
	// 		if(error.r < SHADING_ERROR_THRESHOLD && error.g < SHADING_ERROR_THRESHOLD && error.b < SHADING_ERROR_THRESHOLD){
	// 			break;
	// 		}
	// 	}
	// }

	// int idx = v * camera.imgWidth + u;
	// // TODO: Tone mapping
	// // img[idx] = Color24(camera.sRGB ? ACESFitted(mean) : mean);
	// img[idx] = Color24(camera.sRGB ? mean.Linear2sRGB() : mean);
	// // img[idx] = cyColor24(mean);
	// zimg[idx] = z;
	// sampleCountImg[v * camera.imgWidth + u] = nSamples;
	// renderImage.IncrementNumRenderPixel(1);

	// if (renderImage.IsRenderDone()) {
	// 	std::cout << "Saved render: " << RESOURCES_PATH "img/AB/out.png" << std::endl;
	// 	renderImage.SaveImage(RESOURCES_PATH "img/AB/out.png");
	// }
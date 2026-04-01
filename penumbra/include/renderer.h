#pragma once

#include <iostream>
#include <thread>
#include <filesystem>
#include <cstdio>

#include "minipbrt.h"
#include <OpenImageIO/imageio.h>

#include <sstream>
#include <string>
#include <iomanip>

#include "scene.h"
#include "threading.h"
#include "pbrtconverter.h"
#include "sampling.h"
#ifndef HEADLESS_MODE
#include "gui.h"
#else
// Minimal RenderSettings stub for headless mode (no GUI dependency)
struct RenderSettings {
    int width = 960;
    int height = 540;
    int spp = 1;
    bool indirect = true;
    bool mis = true;
    bool renderLights = false;
    bool renderStereo = false;
    float stereoIPD = 0.065f;
    bool envMapEnabled = false;
    float envMapIntensity = 0.5f;
    bool gammaCorrect = true;
    bool tonemap = true;
    float exposureBias = 1.0f;
};
#endif
#include "color.h"
#include "environmentmap.h"

// TODO: Adaptive sampling 
// #define MIN_SPP 1 
// #define MAX_SPP 8 
// #define SHADING_ERROR_THRESHOLD 1e-1f
// #define LIGHTING_ERROR_THRESHOLD 1e-1f

class BVH;

#define OCCLUDED_EPS 1e-4f
// TODO: Multithread toggle in GUI
 inline unsigned int NTHREADS = (int)std::thread::hardware_concurrency();
 //inline unsigned int NTHREADS = 2;

class Renderer{
public:
    Renderer();
    ~Renderer();
    bool SetPbrtScene(minipbrt::Scene* scene);
    void RenderPixel(int u, int v);
    bool TraceRay(const Ray& ray, HitInfo& hit) const;
    bool Occluded(const glm::vec3& o, const glm::vec3& d, const glm::vec3& n, float maxDist) const ;
    glm::vec3 TracePath(const Ray& ray, Sampler& sampler, int depth, glm::vec3 throughput = glm::vec3(1.0f), bool lastBounceDiffuse = false);
    void RenderAnimation();
    void BeginRender();
    void RenderHeadless(const RenderSettings& rs);
    void StopRender();
    bool LoadScene(const std::string& filename);
    int GetRenderWidth() const { return renderWidth; }
    int GetRenderHeight() const { return renderHeight; }
    void SetRenderWidth(int w) { renderWidth = w; }
    void SetRenderHeight(int h) { renderHeight = h; }
    std::vector<uint8_t>& GetRenderBuffer() { return renderBuffer; }
#ifndef HEADLESS_MODE
    void SetGUI(GUI* guiPtr) { gui = guiPtr; }
#endif
	bool SaveImage();
    void PrintStats();


    // --- GUI Variables (defaults not considered) ---
    char scenePath[256] = "";
    char imgOutPath[256] = "";
    char imgName[256] = "";

private:
    BVH* bvh = nullptr;
    EnvironmentMap envMap;
    std::vector<uint8_t> renderBuffer;
    std::unique_ptr<Scene> scene;
    minipbrt::Scene* pbrtScene = nullptr;
    std::unique_ptr<RenderThreadPool> threadPool;
	bool renderingLeftEye = false;


    // --- GUI Variables (defaults not considered) ---

    // Rendering
#ifndef HEADLESS_MODE
    GUI* gui = nullptr;
#endif
    bool indirectLighting = false;
    bool misEnabled = false;
    int spp = -1;
    int renderWidth = -1;
    int renderHeight = -1;
    bool renderLights = false;

    // Stereo
	bool renderStereo = false;
    float stereoIPD = -1.0f;

    // Env Map
	bool envMapEnabled = false;
	float envMapIntensity = -1.0f;

    // Color
    bool gammaCorrect = false;
    bool tonemap = false;
    float exposureBias = 0.0f;

    // Animation
    char animPath[256] = "";
    char animSavePath[256] = "";

    void ConvertPbrtScene();
};
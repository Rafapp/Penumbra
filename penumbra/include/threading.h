#pragma once

#include <vector>
#include <thread>
#include <atomic>
#include <functional>
#include <mutex>
#include <random>
#include <iostream>
#include <algorithm>
#include <iomanip>
#include <chrono>

class Scene;
class Renderer;

#define TILESIZE 16 
#define SHUFFLE false
#define MORTON_ORDERING false // TODO: Investigate why it's slower? 

class RenderThreadPool {
public:
    RenderThreadPool(Scene* scene, int nThreads, int w, int h)
        : scene(scene), nThreads(nThreads), w(w), h(h),
        activeThreads(0), stop(false), tiles(0), tilesW(0), tilesH(0) {
        tileSize = TILESIZE;
    }

    ~RenderThreadPool();

    void Start(std::function<void(int, int)> render);
    void Stop();
    void Reset();
    void PrintStats();
    std::chrono::steady_clock::time_point startTime;
    std::atomic<bool> frameFinished;

private:
    void RenderWorker(std::function<void(int, int)> render);
    void PrecomputeMortonOrder();

    Scene* scene;
    int nThreads;
    int w, h;

    std::mutex printMutex;
    std::vector<std::thread> workers;
    std::atomic<int> activeThreads;
    std::atomic<bool> stop;

    // Tiling system
    std::vector<std::vector<int>> grid; // Maps a tile to its pixel indices
    std::atomic<int> tiles; 
    int tilesW, tilesH;
    int tileSize = TILESIZE; // Power of two 
};
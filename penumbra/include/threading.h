#pragma once

#include <vector>
#include <thread>
#include <atomic>
#include <functional>
#include <mutex>
#include <random>
#include <iostream>

class Scene;
class Renderer;

#define TILESIZE 8 
#define SHUFFLE false  // TODO: Address morton ordering with shuffling
#define MORTON_ORDERING true 

class RenderThreadPool {
public:
    RenderThreadPool(Scene* scene, int nThreads, int w, int h);
    ~RenderThreadPool();

    void Start(std::function<void(int, int)> render);
    void Stop();
    void Reset();

private:
    void RenderWorker(std::function<void(int, int)> render);
    void PrecomputeMortonOrder();

    Scene* scene;
    int nThreads;
    int w, h;

    std::mutex printMutex;
    std::vector<std::thread> workers;
    std::atomic<int> activeThreads{ 0 };
    std::atomic<bool> stop{ false };

    // Tiling system
    std::vector<std::vector<int>> grid; // Maps a tile to its pixel indices
    std::atomic<int> tiles = {0}; 
    int tilesW, tilesH;
    int tileSize = TILESIZE; // Power of two 
};
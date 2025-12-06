#include "threading.h"
#include <random>
#include <algorithm>

RenderThreadPool::RenderThreadPool(Scene* scene, int nThreads, int w, int h)
    : scene(scene), nThreads(nThreads), w(w), h(h) {
}

RenderThreadPool::~RenderThreadPool() { Stop(); }

// Helper function: Generates archimedean spiral grid
std::vector<std::vector<int>> GenerateSpiralTilemap(int w, int h, int tileSize) {
    enum direction { right, down, left, up };
    direction dir = right;

    const int tilesW = (w + tileSize - 1) / tileSize;
    const int tilesH = (h + tileSize - 1) / tileSize;

    int tx = (tilesW - 1) / 2;
    int ty = (tilesH - 1) / 2;

    std::vector<std::vector<int>> tiles(tilesW * tilesH);
    std::vector<int> order;

    unsigned int iter = 0;
    unsigned int turns = 0;
    unsigned int len = 1;

    while (iter < tilesW * tilesH) {
        if (ty >= 0 && ty < tilesH && tx >= 0 && tx < tilesW) {
            int tileIndex = ty * tilesW + tx;
            order.push_back(tileIndex);
            for (int px = tx * tileSize; px < tx * tileSize + tileSize; px++) {
                for (int py = ty * tileSize; py < ty * tileSize + tileSize; py++) {
                    if (px < w && py < h)
                        tiles[tileIndex].push_back(py * w + px);
                }
            }
            if(SHUFFLE) std::shuffle(tiles[tileIndex].begin(), tiles[tileIndex].end(), std::mt19937{ std::random_device{}() });
            iter++;
        }

        switch (dir) {
        case right:
            tx++;
            if (--len == 0) { dir = down; turns++; len = (turns / 2) + 1; }
            break;
        case down:
            ty++;
            if (--len == 0) { dir = left; turns++; len = (turns / 2) + 1; }
            break;
        case left:
            tx--;
            if (--len == 0) { dir = up; turns++; len = (turns / 2) + 1; }
            break;
        case up:
            ty--;
            if (--len == 0) { dir = right; turns++; len = (turns / 2) + 1; }
            break;
        }
    }

    std::vector<std::vector<int>> spiralTiles;
    spiralTiles.reserve(order.size());
    for (int i : order)
        spiralTiles.push_back(std::move(tiles[i]));

    return spiralTiles;
}

static inline int MortonEncode2D(int x, int y) {
    int morton = 0;
    for (int i = 0; i < 16; i++) {
        morton |= ((x & (1 << i)) << i) | ((y & (1 << i)) << (i + 1));
    }
    return morton;
}

// Morton (Z-order) encoding for better cache coherence
// https://en.wikipedia.org/wiki/Z-order_curve
void RenderThreadPool::PrecomputeMortonOrder() {
    for (auto& tile_pixels : grid) {
        std::sort(tile_pixels.begin(), tile_pixels.end(), [this](int a, int b) {
            int ax = a % w;
            int ay = a / w;
            int bx = b % w;
            int by = b / w;
            return MortonEncode2D(ax, ay) < MortonEncode2D(bx, by);
        });
    }
}

void RenderThreadPool::RenderWorker(std::function<void(int, int)> render) {
    {
        std::lock_guard<std::mutex> lock(printMutex);
        int currenttotal = ++activeThreads;
        std::cout << "dispatching thread pid " << std::this_thread::get_id() 
                  << ", total: " << currenttotal << std::endl;
    }
    int idx;
    while (!stop && (idx = tiles.fetch_add(1, std::memory_order_relaxed)) < tilesW * tilesH) {
        std::vector<int>& pixels = grid[idx];
        for(int pixelidx : pixels){
            int u = pixelidx % w;
            int v = pixelidx / w;
            render(u, v);
        }
    }
    {
        std::lock_guard<std::mutex> lock(printMutex);
        int currenttotal = --activeThreads;
        std::cout << "thread finished pid " << std::this_thread::get_id() 
                  << ", total: " << currenttotal << std::endl;
    }
}

void RenderThreadPool::Start(std::function<void(int, int)> render) {
    std::cout << "Generating tilemap..." << std::endl; 
    tiles = 0;
    grid = GenerateSpiralTilemap(w, h, tileSize);
    if(MORTON_ORDERING) PrecomputeMortonOrder();

    // TODO: Avoid this code repetition
    tilesW = (w + tileSize - 1) / tileSize;
    tilesH = (h + tileSize - 1) / tileSize;

	std::cout << "Dispatching " << nThreads - 1 << " threads." << std::endl;
    stop = false;

    for (int i = 0; i < nThreads - 1; i++) {
        workers.emplace_back(&RenderThreadPool::RenderWorker, this, render);
    }
}

void RenderThreadPool::Stop() {
    stop = true;
    for (auto& t : workers) {
        if (t.joinable()) t.join();
    }
    workers.clear();
}

void RenderThreadPool::Reset() {
    for (auto& t : workers) {
        if (t.joinable()) t.join();
    }
    activeThreads = 0;
    stop = false;
    tiles = 0;
    workers.clear();
}

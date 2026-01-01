#include "threading.h"

//RenderThreadPool::RenderThreadPool(Scene* scene, int nThreads, int w, int h)
//    : scene(scene), nThreads(nThreads), w(w), h(h),
//    activeThreads(0), stop(false), tiles(0), tileSize(TILESIZE),
//    tilesW(0), tilesH(0) {
//    // Force body to exist
//    (void)this;
//}

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
        if(currenttotal == 0){
            PrintStats();
            frameFinished = true;
        }
    }
}

void RenderThreadPool::PrintStats() {
    constexpr bool USE_COLOR = true;
    auto c = [&](const char* code) -> const char* { return USE_COLOR ? code : ""; };

    constexpr const char* RST  = "\x1b[0m";
    constexpr const char* BLD  = "\x1b[1m";
    constexpr const char* DIM  = "\x1b[2m";

    constexpr const char* LINE = "\x1b[38;5;239m";
    constexpr const char* KEY  = "\x1b[38;5;250m";
    constexpr const char* NUM  = "\x1b[38;5;111m";
    constexpr const char* OK   = "\x1b[38;5;120m";
    constexpr const char* BAD  = "\x1b[38;5;203m";
    constexpr const char* ACC  = "\x1b[38;5;81m";

    auto yn = [&](bool v) -> std::string {
        return std::string(c(v ? OK : BAD)) + (v ? "ENABLED" : "DISABLED") + c(RST);
    };

    auto fmtDuration = [](long long msTotal) -> std::string {
        using ll = long long;
        if (msTotal < 0) msTotal = 0;

        const ll ms  = msTotal % 1000;
        const ll sT  = msTotal / 1000;
        const ll s   = sT % 60;
        const ll mT  = sT / 60;
        const ll m   = mT % 60;
        const ll hT  = mT / 60;
        const ll h   = hT % 24;
        const ll d   = hT / 24;

        std::ostringstream oss;
        oss << std::setfill('0');

        if (d > 0) {
            oss << d << "d "
                << std::setw(2) << h << ":";
        } else {
            oss << std::setw(2) << hT << ":";
        }

        oss << std::setw(2) << m << ":"
            << std::setw(2) << s << "."
            << std::setw(3) << ms;

        return oss.str();
    };

    constexpr int KW = 22;
    constexpr int INNER = 51;
    constexpr char HCH = '-';

    const auto endTime  = std::chrono::steady_clock::now();
    const auto msTotal  = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();

    const int tilesRendered = tiles.load(std::memory_order_relaxed);
    const int tilesTotal    = tilesW * tilesH;
    const int threadsActive = activeThreads.load(std::memory_order_relaxed);

    const std::string hdr = "Thread Pool";

    std::cout
        << "\n"
        << "  " << c(BLD) << c(OK) << "RENDER COMPLETE" << c(RST) << "\n"
        << c(LINE) << "  ┌" << std::string(INNER, HCH) << "┐" << c(RST) << "\n"
        << c(LINE) << "  │ " << c(BLD) << c(ACC) << hdr << c(RST)
        << c(LINE) << std::string(std::max(0, INNER - 1 - static_cast<int>(hdr.size())), ' ')
        << "│" << c(RST) << "\n"
        << c(LINE) << "  └" << std::string(INNER, HCH) << "┘" << c(RST) << "\n";

    std::cout << "  " << c(KEY) << std::left << std::setw(KW) << "Render time"
              << c(RST) << c(DIM) << " : " << c(RST)
              << c(NUM) << fmtDuration(static_cast<long long>(msTotal)) << c(RST)
              << c(DIM) << "  (" << c(RST) << c(NUM) << msTotal << c(RST) << c(DIM) << " ms)" << c(RST)
              << "\n";

    std::cout << "  " << c(KEY) << std::left << std::setw(KW) << "Shuffle tiles"
              << c(RST) << c(DIM) << " : " << c(RST)
              << yn(SHUFFLE) << "\n";

    std::cout << "  " << c(KEY) << std::left << std::setw(KW) << "Morton ordering"
              << c(RST) << c(DIM) << " : " << c(RST)
              << yn(MORTON_ORDERING) << "\n";
}

void RenderThreadPool::Start(std::function<void(int, int)> render) {
    tiles = 0;
    grid = GenerateSpiralTilemap(w, h, tileSize);
    if(MORTON_ORDERING) PrecomputeMortonOrder();

    // TODO: Avoid this code repetition
    tilesW = (w + tileSize - 1) / tileSize;
    tilesH = (h + tileSize - 1) / tileSize;

	std::cout << "Rendering with " << nThreads - 1 << " threads ..." << std::endl;
    stop = false;
    frameFinished = false;

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

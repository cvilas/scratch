#include <SDL3/SDL.h>

#define STB_IMAGE_IMPLEMENTATION
#include "third_party/stb_image.h"

#include <httplib.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <condition_variable>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <deque>
#include <filesystem>
#include <format>
#include <fstream>
#include <list>
#include <mutex>
#include <optional>
#include <string>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace fs = std::filesystem;

namespace {

constexpr int TILE_SIZE = 256;
constexpr double MIN_LAT = -85.05112878;
constexpr double MAX_LAT = 85.05112878;
constexpr int MIN_ZOOM = 1;
constexpr int MAX_ZOOM = 19;

constexpr std::size_t RAM_BUDGET_MB = 192;
constexpr std::size_t RAM_BUDGET_BYTES = RAM_BUDGET_MB * 1024ull * 1024ull;
constexpr int WORKER_COUNT = 4;

double clamp(double v, double lo, double hi) {
    return std::max(lo, std::min(hi, v));
}

double wrap_lon(double lon) {
    while (lon < -180.0) lon += 360.0;
    while (lon >= 180.0) lon -= 360.0;
    return lon;
}

struct TileKey {
    int z{};
    int x{};
    int y{};

    bool operator==(const TileKey& o) const noexcept {
        return z == o.z && x == o.x && y == o.y;
    }
};

struct TileKeyHash {
    std::size_t operator()(const TileKey& k) const noexcept {
        std::size_t h1 = std::hash<int>{}(k.z);
        std::size_t h2 = std::hash<int>{}(k.x);
        std::size_t h3 = std::hash<int>{}(k.y);
        return h1 ^ (h2 << 1) ^ (h3 << 2);
    }
};

struct TilePixels {
    int w{};
    int h{};
    std::vector<unsigned char> rgba;
    std::size_t bytes() const { return rgba.size(); }
};

struct DecodedReady {
    TileKey key{};
    TilePixels px;
};

double lon_to_world_x(double lon_deg, int z) {
    const double n = std::ldexp(1.0, z);
    return ((wrap_lon(lon_deg) + 180.0) / 360.0) * n * TILE_SIZE;
}

double lat_to_world_y(double lat_deg, int z) {
    lat_deg = clamp(lat_deg, MIN_LAT, MAX_LAT);
    const double lat_rad = lat_deg * M_PI / 180.0;
    const double n = std::ldexp(1.0, z);
    const double y = (1.0 - std::log(std::tan(lat_rad) + 1.0 / std::cos(lat_rad)) / M_PI) / 2.0;
    return y * n * TILE_SIZE;
}

double world_x_to_lon(double wx, int z) {
    const double n = std::ldexp(1.0, z) * TILE_SIZE;
    return (wx / n) * 360.0 - 180.0;
}

double world_y_to_lat(double wy, int z) {
    const double n = std::ldexp(1.0, z) * TILE_SIZE;
    const double yy = M_PI * (1.0 - 2.0 * (wy / n));
    const double lat_rad = std::atan(std::sinh(yy));
    return lat_rad * 180.0 / M_PI;
}

class DiskCache {
public:
    explicit DiskCache(fs::path root) : root_(std::move(root)) {
        std::error_code ec;
        fs::create_directories(root_, ec);
    }

    fs::path path_for(const TileKey& k) const {
        return root_ / std::to_string(k.z) / std::to_string(k.x) / (std::to_string(k.y) + ".png");
    }

    bool load_png(const TileKey& k, std::vector<unsigned char>& out) const {
        std::ifstream ifs(path_for(k), std::ios::binary);
        if (!ifs) return false;
        out.assign(std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>());
        return !out.empty();
    }

    void store_png(const TileKey& k, const std::vector<unsigned char>& bytes) {
        fs::path p = path_for(k);
        std::error_code ec;
        fs::create_directories(p.parent_path(), ec);
        std::ofstream ofs(p, std::ios::binary);
        if (!ofs) return;
        ofs.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    }

private:
    fs::path root_;
};

class RamLru {
public:
    struct Node {
        TileKey key{};
        TilePixels px;
    };

    explicit RamLru(std::size_t budget) : budget_(budget) {}

    bool get(const TileKey& k, TilePixels& out) {
        auto it = map_.find(k);
        if (it == map_.end()) return false;
        touch(it->second);
        out = it->second->px;
        return true;
    }

    void put(const TileKey& k, TilePixels px) {
        const std::size_t b = px.bytes();
        if (b > budget_) return;

        auto it = map_.find(k);
        if (it != map_.end()) {
            used_ -= it->second->px.bytes();
            it->second->px = std::move(px);
            used_ += b;
            touch(it->second);
            evict();
            return;
        }

        lru_.push_front(Node{k, std::move(px)});
        map_[k] = lru_.begin();
        used_ += b;
        evict();
    }

private:
    using It = std::list<Node>::iterator;

    void touch(It it) {
        if (it == lru_.begin()) return;
        lru_.splice(lru_.begin(), lru_, it);
    }

    void evict() {
        while (used_ > budget_ && !lru_.empty()) {
            auto it = std::prev(lru_.end());
            used_ -= it->px.bytes();
            map_.erase(it->key);
            lru_.erase(it);
        }
    }

    std::size_t budget_;
    std::size_t used_ = 0;
    std::list<Node> lru_;
    std::unordered_map<TileKey, It, TileKeyHash> map_;
};

struct Camera {
    double lat = 37.7749;
    double lon = -122.4194;
    int zoom = 4;
};

struct TileTask {
    TileKey key{};
    int priority = 0; // lower = more important
};

class TaskQueue {
public:
    void push(TileTask t) {
        std::scoped_lock lk(m_);
        // De-dupe queued tasks
        for (auto const& e : q_) {
            if (e.key == t.key) return;
        }
        q_.push_back(t);
        cv_.notify_one();
    }

    std::optional<TileTask> pop_wait(std::atomic<bool>& stop) {
        std::unique_lock lk(m_);
        cv_.wait(lk, [&] { return stop.load() || !q_.empty(); });
        if (stop.load()) return std::nullopt;

        // pick best priority (simple linear search)
        auto best_it = q_.begin();
        for (auto it = q_.begin(); it != q_.end(); ++it) {
            if (it->priority < best_it->priority) best_it = it;
        }
        TileTask t = *best_it;
        q_.erase(best_it);
        return t;
    }

    void wake_all() { cv_.notify_all(); }

private:
    std::mutex m_;
    std::condition_variable cv_;
    std::deque<TileTask> q_;
};

TilePixels decode_rgba(const unsigned char* data, int len) {
    TilePixels out;
    int w = 0, h = 0, ch = 0;
    unsigned char* px = stbi_load_from_memory(data, len, &w, &h, &ch, 4);
    if (!px) return out;
    out.w = w;
    out.h = h;
    out.rgba.assign(px, px + (w * h * 4));
    stbi_image_free(px);
    return out;
}

std::string tile_path(const TileKey& k) {
    return std::format("/{}/{}/{}.png", k.z, k.x, k.y);
}

} // namespace

int main() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("SDL3 Map PoC v2", 1280, 800, SDL_WINDOW_RESIZABLE);
    if (!window) {
        std::fprintf(stderr, "CreateWindow failed: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);
    if (!renderer) {
        std::fprintf(stderr, "CreateRenderer failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    Camera cam{};

    // RAM decoded cache
    RamLru ram_cache(RAM_BUDGET_BYTES);
    std::mutex ram_mtx;

    // GPU texture map (not LRU-managed in this minimal v2; can be added in v3)
    std::unordered_map<TileKey, SDL_Texture*, TileKeyHash> gpu_tiles;
    std::mutex gpu_mtx;

    // in-flight tracking
    std::unordered_set<TileKey, TileKeyHash> in_flight;
    std::mutex inflight_mtx;

    // decoded ready queue (worker -> main thread)
    std::deque<DecodedReady> ready_q;
    std::mutex ready_mtx;

    // disk cache under ~/.cache/sdl3-map-poc
    const char* home = std::getenv("HOME");
    fs::path cache_root = (home ? fs::path(home) : fs::temp_directory_path()) / ".cache" / "sdl3-map-poc";
    DiskCache disk(cache_root);

    TaskQueue tasks;
    std::atomic<bool> stop{false};

    // workers
    std::vector<std::thread> workers;
    workers.reserve(WORKER_COUNT);

    for (int i = 0; i < WORKER_COUNT; ++i) {
        workers.emplace_back([&] {
#if defined(CPPHTTPLIB_OPENSSL_SUPPORT)
            httplib::Client cli("https://tile.openstreetmap.org");
#else
            httplib::Client cli("http://tile.openstreetmap.org");
#endif
            cli.set_follow_location(true);
            cli.set_connection_timeout(5, 0);
            cli.set_read_timeout(10, 0);
            cli.set_write_timeout(10, 0);

            while (!stop.load()) {
                auto maybe = tasks.pop_wait(stop);
                if (!maybe) break;
                TileKey k = maybe->key;

                // 1) RAM cache hit?
                {
                    std::scoped_lock lk(ram_mtx);
                    TilePixels px;
                    if (ram_cache.get(k, px)) {
                        std::scoped_lock lk2(ready_mtx);
                        ready_q.push_back(DecodedReady{k, std::move(px)});
                        continue;
                    }
                }

                // 2) Disk cache?
                std::vector<unsigned char> png_bytes;
                if (disk.load_png(k, png_bytes)) {
                    TilePixels px = decode_rgba(png_bytes.data(), static_cast<int>(png_bytes.size()));
                    if (!px.rgba.empty()) {
                        {
                            std::scoped_lock lk(ram_mtx);
                            ram_cache.put(k, px);
                        }
                        std::scoped_lock lk2(ready_mtx);
                        ready_q.push_back(DecodedReady{k, std::move(px)});
                        continue;
                    }
                }

                // 3) Network
                auto res = cli.Get(tile_path(k).c_str());
                if (!res || res->status != 200 || res->body.empty()) {
                    std::scoped_lock lk(inflight_mtx);
                    in_flight.erase(k);
                    continue;
                }

                std::vector<unsigned char> net_bytes(res->body.begin(), res->body.end());
                TilePixels px = decode_rgba(net_bytes.data(), static_cast<int>(net_bytes.size()));
                if (px.rgba.empty()) {
                    std::scoped_lock lk(inflight_mtx);
                    in_flight.erase(k);
                    continue;
                }

                // store disk + ram
                disk.store_png(k, net_bytes);
                {
                    std::scoped_lock lk(ram_mtx);
                    ram_cache.put(k, px);
                }

                std::scoped_lock lk2(ready_mtx);
                ready_q.push_back(DecodedReady{k, std::move(px)});
            }
        });
    }

    auto enqueue_tile = [&](const TileKey& k, int priority) {
        {
            std::scoped_lock lk(gpu_mtx);
            if (gpu_tiles.find(k) != gpu_tiles.end()) return;
        }
        {
            std::scoped_lock lk(inflight_mtx);
            if (in_flight.contains(k)) return;
            in_flight.insert(k);
        }
        tasks.push(TileTask{k, priority});
    };

    bool running = true;
    bool dragging = false;
    float last_x = 0.f;
    float last_y = 0.f;

    while (running) {
        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_EVENT_QUIT) running = false;

            if (ev.type == SDL_EVENT_KEY_DOWN && ev.key.key == SDLK_ESCAPE) {
                running = false;
            }

            if (ev.type == SDL_EVENT_MOUSE_BUTTON_DOWN && ev.button.button == SDL_BUTTON_LEFT) {
                dragging = true;
                last_x = ev.button.x;
                last_y = ev.button.y;
            }

            if (ev.type == SDL_EVENT_MOUSE_BUTTON_UP && ev.button.button == SDL_BUTTON_LEFT) {
                dragging = false;
            }

            if (ev.type == SDL_EVENT_MOUSE_MOTION && dragging) {
                float dx = ev.motion.x - last_x;
                float dy = ev.motion.y - last_y;
                last_x = ev.motion.x;
                last_y = ev.motion.y;

                double cx = lon_to_world_x(cam.lon, cam.zoom);
                double cy = lat_to_world_y(cam.lat, cam.zoom);
                cx -= dx;
                cy -= dy;
                cam.lon = world_x_to_lon(cx, cam.zoom);
                cam.lat = clamp(world_y_to_lat(cy, cam.zoom), MIN_LAT, MAX_LAT);
            }

            if (ev.type == SDL_EVENT_MOUSE_WHEEL) {
                if (ev.wheel.y > 0) cam.zoom = std::min(cam.zoom + 1, MAX_ZOOM);
                if (ev.wheel.y < 0) cam.zoom = std::max(cam.zoom - 1, MIN_ZOOM);
            }
        }

        // upload ready tiles (main thread)
        {
            std::scoped_lock lk(ready_mtx);
            while (!ready_q.empty()) {
                auto item = std::move(ready_q.front());
                ready_q.pop_front();

                SDL_Surface* surf = SDL_CreateSurfaceFrom(
                    item.px.w, item.px.h, SDL_PIXELFORMAT_RGBA32,
                    item.px.rgba.data(), item.px.w * 4
                );
                if (surf) {
                    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
                    SDL_DestroySurface(surf);
                    if (tex) {
                        std::scoped_lock lk2(gpu_mtx);
                        auto it = gpu_tiles.find(item.key);
                        if (it == gpu_tiles.end()) {
                            gpu_tiles[item.key] = tex;
                        } else {
                            SDL_DestroyTexture(tex); // already present
                        }
                    }
                }

                std::scoped_lock lk3(inflight_mtx);
                in_flight.erase(item.key);
            }
        }

        int ww = 0, wh = 0;
        SDL_GetWindowSize(window, &ww, &wh);

        const double cx = lon_to_world_x(cam.lon, cam.zoom);
        const double cy = lat_to_world_y(cam.lat, cam.zoom);
        const double tlx = cx - ww * 0.5;
        const double tly = cy - wh * 0.5;

        const int tiles_per_axis = (1 << cam.zoom);
        const int max_tile = tiles_per_axis - 1;

        const int start_tx = static_cast<int>(std::floor(tlx / TILE_SIZE));
        const int start_ty = static_cast<int>(std::floor(tly / TILE_SIZE));
        const int end_tx = static_cast<int>(std::floor((tlx + ww) / TILE_SIZE));
        const int end_ty = static_cast<int>(std::floor((tly + wh) / TILE_SIZE));

        SDL_SetRenderDrawColor(renderer, 15, 15, 18, 255);
        SDL_RenderClear(renderer);

        struct Needed {
            TileKey key;
            int priority;
            SDL_FRect dst;
        };
        std::vector<Needed> needed;

        // Build list and priorities (center-first)
        const int c_tx = static_cast<int>(std::floor(cx / TILE_SIZE));
        const int c_ty = static_cast<int>(std::floor(cy / TILE_SIZE));

        for (int ty = start_ty; ty <= end_ty; ++ty) {
            if (ty < 0 || ty > max_tile) continue;
            for (int tx = start_tx; tx <= end_tx; ++tx) {
                int wrapped_tx = tx;
                while (wrapped_tx < 0) wrapped_tx += tiles_per_axis;
                while (wrapped_tx >= tiles_per_axis) wrapped_tx -= tiles_per_axis;

                TileKey k{cam.zoom, wrapped_tx, ty};
                int dx = tx - c_tx;
                int dy = ty - c_ty;
                int prio = dx * dx + dy * dy; // smaller first

                float sx = static_cast<float>(tx * TILE_SIZE - tlx);
                float sy = static_cast<float>(ty * TILE_SIZE - tly);
                SDL_FRect dst{sx, sy, static_cast<float>(TILE_SIZE), static_cast<float>(TILE_SIZE)};
                needed.push_back(Needed{k, prio, dst});
            }
        }

        std::sort(needed.begin(), needed.end(), [](const Needed& a, const Needed& b) {
            return a.priority < b.priority;
        });

        for (auto const& n : needed) {
            bool drawn = false;
            {
                std::scoped_lock lk(gpu_mtx);
                auto it = gpu_tiles.find(n.key);
                if (it != gpu_tiles.end() && it->second) {
                    SDL_RenderTexture(renderer, it->second, nullptr, &n.dst);
                    drawn = true;
                }
            }

            if (!drawn) {
                enqueue_tile(n.key, n.priority);
                SDL_SetRenderDrawColor(renderer, 45, 45, 48, 255);
                SDL_RenderFillRect(renderer, &n.dst);
                SDL_SetRenderDrawColor(renderer, 70, 70, 76, 255);
                SDL_RenderRect(renderer, &n.dst);
            }
        }

        // center crosshair
        SDL_SetRenderDrawColor(renderer, 255, 80, 80, 255);
        SDL_FRect h{ww * 0.5f - 10.f, wh * 0.5f, 20.f, 1.f};
        SDL_FRect v{ww * 0.5f, wh * 0.5f - 10.f, 1.f, 20.f};
        SDL_RenderFillRect(renderer, &h);
        SDL_RenderFillRect(renderer, &v);

        SDL_RenderPresent(renderer);
    }

    stop = true;
    tasks.wake_all();

    for (auto& t : workers) {
        if (t.joinable()) t.join();
    }

    {
        std::scoped_lock lk(gpu_mtx);
        for (auto& [_, tex] : gpu_tiles) {
            if (tex) SDL_DestroyTexture(tex);
        }
        gpu_tiles.clear();
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

#include <SDL3/SDL.h>

#define STB_IMAGE_IMPLEMENTATION
#include "third_party/stb_image.h"

#include <httplib.h>

#include <algorithm>
#include <atomic>
#include <cmath>
#include <condition_variable>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <deque>
#include <format>
#include <mutex>
#include <optional>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

namespace {

    constexpr int TILE_SIZE = 256;
    constexpr double MIN_LAT = -85.05112878;
    constexpr double MAX_LAT =  85.05112878;
    constexpr int MIN_ZOOM = 1;
    constexpr int MAX_ZOOM = 19;

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

    struct DecodedTile {
        TileKey key{};
        int w{};
        int h{};
        std::vector<unsigned char> rgba;
    };

    struct TileEntry {
        SDL_Texture* texture = nullptr;
    };

    struct Camera {
        double lat = 37.7749;   // San Francisco default
        double lon = -122.4194;
        int zoom = 4;
    };

    double lon_to_world_x(double lon_deg, int z) {
        const double n = std::ldexp(1.0, z); // 2^z
        return ((wrap_lon(lon_deg) + 180.0) / 360.0) * n * TILE_SIZE;
    }

    double lat_to_world_y(double lat_deg, int z) {
        lat_deg = clamp(lat_deg, MIN_LAT, MAX_LAT);
        double lat_rad = lat_deg * M_PI / 180.0;
        const double n = std::ldexp(1.0, z);
        double y = (1.0 - std::log(std::tan(lat_rad) + 1.0 / std::cos(lat_rad)) / M_PI) / 2.0;
        return y * n * TILE_SIZE;
    }

    double world_x_to_lon(double wx, int z) {
        const double n = std::ldexp(1.0, z) * TILE_SIZE;
        return (wx / n) * 360.0 - 180.0;
    }

    double world_y_to_lat(double wy, int z) {
        const double n = std::ldexp(1.0, z) * TILE_SIZE;
        double yy = M_PI * (1.0 - 2.0 * (wy / n));
        double lat_rad = std::atan(std::sinh(yy));
        return lat_rad * 180.0 / M_PI;
    }

    class DownloadQueue {
    public:
        void push(TileKey k) {
            std::scoped_lock lk(m_);
            // de-dupe queued items
            for (auto& e : q_) if (e == k) return;
            q_.push_back(k);
            cv_.notify_one();
        }

        std::optional<TileKey> pop_wait(std::atomic<bool>& stop) {
            std::unique_lock lk(m_);
            cv_.wait(lk, [&] { return stop.load() || !q_.empty(); });
            if (stop.load()) return std::nullopt;
            TileKey k = q_.front();
            q_.pop_front();
            return k;
        }

    private:
        std::mutex m_;
        std::condition_variable cv_;
        std::deque<TileKey> q_;
    };

    std::string tile_path(const TileKey& k) {
        return std::format("/{}/{}/{}.png", k.z, k.x, k.y);
    }

} // namespace

int main() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("SDL3 Map PoC", 1280, 800, SDL_WINDOW_RESIZABLE);
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
    bool running = true;
    bool dragging = false;
    float last_mouse_x = 0.f;
    float last_mouse_y = 0.f;

    std::unordered_map<TileKey, TileEntry, TileKeyHash> tiles;
    std::mutex tiles_mtx;

    DownloadQueue dlq;
    std::deque<DecodedTile> decoded_ready;
    std::mutex decoded_mtx;

    std::unordered_map<TileKey, bool, TileKeyHash> in_flight;
    std::mutex inflight_mtx;

    std::atomic<bool> stop{false};

    // One simple worker thread for minimal PoC
    std::thread worker([&] {
        httplib::Client cli("https://tile.openstreetmap.org");
        cli.set_follow_location(true);
        cli.set_connection_timeout(5, 0);
        cli.set_read_timeout(10, 0);
        cli.set_write_timeout(10, 0);

        while (!stop.load()) {
            auto maybe = dlq.pop_wait(stop);
            if (!maybe) break;
            TileKey k = *maybe;

            auto res = cli.Get(tile_path(k).c_str());
            if (!res || res->status != 200 || res->body.empty()) {
                std::scoped_lock lk(inflight_mtx);
                in_flight.erase(k);
                continue;
            }

            int w = 0, h = 0, ch = 0;
            unsigned char* px = stbi_load_from_memory(
                reinterpret_cast<const unsigned char*>(res->body.data()),
                                                      static_cast<int>(res->body.size()),
                                                      &w, &h, &ch, 4
            );
            if (!px) {
                std::scoped_lock lk(inflight_mtx);
                in_flight.erase(k);
                continue;
            }

            DecodedTile dt;
            dt.key = k;
            dt.w = w;
            dt.h = h;
            dt.rgba.assign(px, px + (w * h * 4));
            stbi_image_free(px);

            {
                std::scoped_lock lk(decoded_mtx);
                decoded_ready.push_back(std::move(dt));
            }
        }
    });

    auto request_tile = [&](const TileKey& k) {
        {
            std::scoped_lock lk(tiles_mtx);
            if (tiles.find(k) != tiles.end()) return;
        }

        std::scoped_lock lk(inflight_mtx);
        if (in_flight.find(k) != in_flight.end()) return;
        in_flight[k] = true;
        dlq.push(k);
    };

    while (running) {
        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_EVENT_QUIT) running = false;

            if (ev.type == SDL_EVENT_MOUSE_BUTTON_DOWN && ev.button.button == SDL_BUTTON_LEFT) {
                dragging = true;
                last_mouse_x = ev.button.x;
                last_mouse_y = ev.button.y;
            }
            if (ev.type == SDL_EVENT_MOUSE_BUTTON_UP && ev.button.button == SDL_BUTTON_LEFT) {
                dragging = false;
            }
            if (ev.type == SDL_EVENT_MOUSE_MOTION && dragging) {
                float dx = ev.motion.x - last_mouse_x;
                float dy = ev.motion.y - last_mouse_y;
                last_mouse_x = ev.motion.x;
                last_mouse_y = ev.motion.y;

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

            if (ev.type == SDL_EVENT_KEY_DOWN) {
                if (ev.key.key == SDLK_ESCAPE) running = false;
            }
        }

        // Upload decoded tiles on render thread
        {
            std::scoped_lock lk(decoded_mtx);
            while (!decoded_ready.empty()) {
                DecodedTile dt = std::move(decoded_ready.front());
                decoded_ready.pop_front();

                SDL_Surface* surf = SDL_CreateSurfaceFrom(
                    dt.w, dt.h,
                    SDL_PIXELFORMAT_RGBA32,
                    dt.rgba.data(),
                                                          dt.w * 4
                );
                if (surf) {
                    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
                    SDL_DestroySurface(surf);
                    if (tex) {
                        std::scoped_lock lk2(tiles_mtx);
                        tiles[dt.key] = TileEntry{tex};
                    }
                }

                std::scoped_lock lk3(inflight_mtx);
                in_flight.erase(dt.key);
            }
        }

        int ww = 0, wh = 0;
        SDL_GetWindowSize(window, &ww, &wh);

        const double center_world_x = lon_to_world_x(cam.lon, cam.zoom);
        const double center_world_y = lat_to_world_y(cam.lat, cam.zoom);

        const double top_left_world_x = center_world_x - ww * 0.5;
        const double top_left_world_y = center_world_y - wh * 0.5;

        const int max_tile = (1 << cam.zoom) - 1;
        const int start_tx = static_cast<int>(std::floor(top_left_world_x / TILE_SIZE));
        const int start_ty = static_cast<int>(std::floor(top_left_world_y / TILE_SIZE));
        const int end_tx   = static_cast<int>(std::floor((top_left_world_x + ww) / TILE_SIZE));
        const int end_ty   = static_cast<int>(std::floor((top_left_world_y + wh) / TILE_SIZE));

        SDL_SetRenderDrawColor(renderer, 18, 18, 18, 255);
        SDL_RenderClear(renderer);

        for (int ty = start_ty; ty <= end_ty; ++ty) {
            if (ty < 0 || ty > max_tile) continue;
            for (int tx = start_tx; tx <= end_tx; ++tx) {
                // wrap X around antimeridian
                int wrapped_tx = tx;
                int world_tiles = (1 << cam.zoom);
                while (wrapped_tx < 0) wrapped_tx += world_tiles;
                while (wrapped_tx >= world_tiles) wrapped_tx -= world_tiles;

                TileKey k{cam.zoom, wrapped_tx, ty};

                float sx = static_cast<float>(tx * TILE_SIZE - top_left_world_x);
                float sy = static_cast<float>(ty * TILE_SIZE - top_left_world_y);

                SDL_FRect dst{sx, sy, static_cast<float>(TILE_SIZE), static_cast<float>(TILE_SIZE)};

                bool drawn = false;
                {
                    std::scoped_lock lk(tiles_mtx);
                    auto it = tiles.find(k);
                    if (it != tiles.end() && it->second.texture) {
                        SDL_RenderTexture(renderer, it->second.texture, nullptr, &dst);
                        drawn = true;
                    }
                }

                if (!drawn) {
                    request_tile(k);
                    // placeholder tile
                    SDL_SetRenderDrawColor(renderer, 45, 45, 45, 255);
                    SDL_RenderFillRect(renderer, &dst);
                    SDL_SetRenderDrawColor(renderer, 70, 70, 70, 255);
                    SDL_RenderRect(renderer, &dst);
                }
            }
        }

        // Crosshair at center
        SDL_SetRenderDrawColor(renderer, 255, 64, 64, 255);
        SDL_FRect hline{ww * 0.5f - 12.f, wh * 0.5f, 24.f, 1.f};
        SDL_FRect vline{ww * 0.5f, wh * 0.5f - 12.f, 1.f, 24.f};
        SDL_RenderFillRect(renderer, &hline);
        SDL_RenderFillRect(renderer, &vline);

        SDL_RenderPresent(renderer);
    }

    stop = true;
    dlq.push(TileKey{}); // wake worker
    worker.join();

    {
        std::scoped_lock lk(tiles_mtx);
        for (auto& [_, e] : tiles) {
            if (e.texture) SDL_DestroyTexture(e.texture);
        }
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

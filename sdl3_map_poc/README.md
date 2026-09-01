# SDL3 Map Viewer PoC (C++23 + CMake)

This repository contains two proof-of-concept map viewers built with SDL3:

- **v1 (`main.cpp`)**: minimal slippy-map renderer with async tile download + decode and basic in-memory tile map.
- **v2 (`main_v2.cpp`)**: enhanced PoC with better resource usage via RAM LRU cache, disk tile cache, prioritized loading, and worker pool.

Both implementations use Web Mercator / XYZ map tiles and render map imagery in real time with pan + zoom interactions.

---

## Goals

- Display map tiles centered around GPS coordinates.
- Allow interactive pan and zoom.
- Keep architecture simple but scalable.
- Move toward efficiency across:
  - **CPU**
  - **Memory**
  - **Network**

---

## Common Architecture (v1 and v2)

Both versions share these core concepts:

1. **Camera model**
   - Stores center latitude/longitude and integer zoom.
   - Converts geographic coordinates to world pixel space (Web Mercator).
   - Computes visible tile range from viewport and camera.

2. **Tile identity**
   - Tile key is `(z, x, y)` in XYZ format.
   - Longitude wraps around antimeridian; latitude is clamped to Mercator bounds.

3. **Async pipeline**
   - Worker thread(s) fetch and decode PNG tiles off the render thread.
   - Main thread uploads decoded RGBA buffers into SDL textures (required by SDL threading model).

4. **Renderer**
   - Draws textured quads for available tiles.
   - Draws placeholders for missing tiles.
   - Supports mouse drag pan and mouse wheel zoom.

---

## v1 Design (`src/main.cpp`)

### Summary

v1 is intentionally minimal. It demonstrates end-to-end tile rendering with a single worker and in-memory texture storage.

### Data flow

1. Visible tile list is computed each frame.
2. Missing tiles are queued for download.
3. Worker downloads tile PNG over HTTP(S) and decodes to RGBA.
4. Main thread converts RGBA into `SDL_Texture`.
5. Texture is stored in a hash map for reuse.

### Key characteristics

- **Simple implementation**
  - One worker thread.
  - One in-memory texture map.
  - In-flight request deduplication.
- **Good first baseline**
  - Easy to debug.
  - Easy to extend.

### Limitations

- No cache eviction policy (memory can grow unbounded over time).
- No persistent disk cache.
- No request prioritization beyond queue order.
- Limited network efficiency controls.

---

## v2 Design (`src/main_v2.cpp`)

### Summary

v2 keeps the same rendering/camera model but improves efficiency with explicit caching and load prioritization.

### New components

1. **RAM LRU cache (decoded pixels)**
   - Byte-budgeted cache (`RAM_BUDGET_BYTES`).
   - Evicts least-recently-used decoded tiles when over budget.
   - Reduces repeated decoding/network fetches.

2. **Disk cache (PNG bytes)**
   - Stores raw tile images under:
     - `~/.cache/sdl3-map-poc/<z>/<x>/<y>.png`
   - Provides persistence across runs.
   - Enables cold-start reuse without network.

3. **Worker pool**
   - Multiple workers (`WORKER_COUNT`) process tile tasks concurrently.
   - Better throughput on multi-core systems.

4. **Priority queue behavior**
   - Tile tasks are prioritized center-first using squared distance from viewport center tile.
   - Most visible/important tiles arrive first.

5. **Three-tier tile lookup path**
   - RAM cache -> Disk cache -> Network fetch.

### Data flow (v2)

1. Frame determines needed tiles and their priorities.
2. Missing tiles are enqueued with dedup + priority.
3. Worker checks RAM cache.
4. On miss, worker checks disk PNG.
5. On miss, worker downloads from network and stores to disk.
6. Decoded tile is pushed to main-thread upload queue.
7. Main thread creates SDL texture and renders it.

### Efficiency impact

- **CPU**
  - Less repeated decoding due to RAM cache.
  - Parallel decode/fetch via worker pool.
- **Memory**
  - Decoded RAM bounded by LRU budget.
- **Network**
  - Disk cache avoids repeat downloads across sessions.
  - In-flight dedup avoids duplicate requests during fast panning.

### Remaining limitations (intentional for PoC scope)

- GPU texture cache is not yet LRU-evicted.
- No HTTP revalidation (ETag / If-Modified-Since) yet.
- No cancellation token for stale requests.
- No fractional zoom interpolation (integer zoom steps only).

---

## Coordinate and Tile Model

Both versions use XYZ slippy map equations:

- `n = 2^z`
- `x = n * ((lon + 180) / 360)`
- `y = n * (1 - ln(tan(lat) + sec(lat)) / π) / 2`

Implementation operates in world pixel coordinates:

- `world_x = x * tile_size`
- `world_y = y * tile_size`
- `tile_size = 256`

This allows straightforward viewport-to-tile range calculations.

---

## Threading Model

- **Main thread**
  - SDL event polling
  - Frame rendering
  - Texture creation/destruction

- **Worker thread(s)**
  - Tile fetch (HTTP client)
  - PNG decode (`stb_image`)
  - Enqueue decoded tiles for main-thread upload

This separation keeps frame pacing smoother and avoids render hitches from blocking network/decode tasks.

---

## Next recommended steps (v2.1+)

1. Add GPU texture LRU with byte accounting.
2. Add per-frame upload budget (avoid frame spikes).
3. Add HTTP cache validation (ETag/Last-Modified).
4. Add stale request cancellation when camera changes rapidly.
5. Add optional prefetch ring outside viewport.
6. Add attribution/terms-compliant overlay UI.
7. Add map source abstraction (street/satellite providers).

---

## Why this progression

- **v1** validates correctness and SDL integration quickly.
- **v2** introduces practical systems needed for real usage efficiency.
- Future iterations can remain incremental without re-architecting core map math or rendering flow.

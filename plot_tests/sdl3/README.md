# sdlplot — Real-time Time Series Plotter (SDL3)

A lightweight, single-file real-time plot built on **SDL3** with no other
dependencies.  Designed as a self-contained drop-in for visualising live
sensor data, signal processing output, or any floating-point stream.

---

## Features

| Feature | Detail |
|---|---|
| Real-time scrolling | Ring-buffer stores the last N samples; older data scrolls off the left edge |
| Resizable window | Plot area reflows automatically when the window is resized |
| X zoom | Scroll wheel narrows/widens the visible sample window, pinned to the latest sample |
| Y zoom | Ctrl + scroll wheel zooms the Y range around the cursor's data value |
| Reset zoom | Double-click inside the plot area |
| Scrolling grid | Grid lines are anchored to data-space values (like implot), so they slide with the data rather than staying fixed on screen |
| Smart tick labels | 1/2/5 × power-of-ten step selection keeps labels readable at any zoom level |
| Movable legend | Click and drag the legend box anywhere on the window |
| User-specified labels | X axis title, Y axis title, and series name are set at runtime |

---

## Architecture

```
PlotContext
├── TimeSeries          circular ring-buffer + ever-increasing total_count
├── Legend              position, drag state
├── view_start/view_n   derived each frame from x_visible + total_count
├── x_scale             pixels per sample, derived each frame
├── vy_min / vy_max     Y range (auto or manual after Ctrl-scroll)
└── x_label / y_label / series_name   user-supplied strings
```

### Data flow (per frame)

```
add_datapoint()          push new float into ring buffer, bump total_count
      │
update_view()            compute view_start, view_n, x_scale, vy_min/max
      │
draw_grid()              vertical lines at nice_step(view_n) sample intervals
                         horizontal lines at nice_step(vy_range) value intervals
draw_axes()              left + bottom border lines
draw_timeseries()        polyline over [view_start … view_start+view_n)
                         uses get_value_global() to map global→buffer index
draw_axis_labels()       ticks at the same nice_step positions as the grid
draw_legend()            colour swatch + series_name text, draggable
```

### Key design decisions

**Ring buffer with `total_count`**  
`TimeSeries.count` caps at `capacity` once the buffer is full.
`total_count` never resets and drives `view_start`, keeping the X tick
labels scrolling correctly past sample 400, 500, … indefinitely.

**`get_value_global(ts, g)`**  
Converts a global index `g` to a buffer-local index:
```
local = count + g - total_count
```
This avoids any special-casing in the drawing loop.

**`nice_step(range, n)`**  
Returns the largest 1/2/5 × 10^k step that fits `n` divisions into `range`.
The same function drives both the grid and the tick labels, so they are
always aligned.

**Zoom**  
- X zoom: `x_visible` stores the number of samples to show (`0` = all).
  `zoom_x()` multiplies by a factor and clamps to `[MIN_X_VISIBLE, count]`.
- Y zoom: `y_manual = true` freezes auto-ranging; `zoom_y()` scales
  `[vy_min, vy_max]` around the pivot data value under the cursor.
- `reset_zoom()` clears both.

---

## Build

### Manual (quickest)

```bash
g++ -o sdlplot sdlplot.cpp -lSDL3
./sdlplot
```

### CMake (this directory)

```bash
mkdir build && cd build
cmake ..
cmake --build .
./sdlplot
```

### CMake (parent plot_tests project)

The parent `CMakeLists.txt` already includes this target:

```bash
cd plot_tests
mkdir build && cd build
cmake ..
cmake --build . --target sdlplot
```

---

## Usage

### Controls

| Input | Action |
|---|---|
| Scroll wheel (on plot) | Zoom X in/out (pinned to latest sample) |
| Ctrl + scroll wheel | Zoom Y in/out (around cursor value) |
| Double-click (on plot) | Reset both axes to auto |
| Drag legend | Move the legend box anywhere |
| Resize window | Plot area reflows; grid and labels update |

### Integrating your own data source

Replace `generate_sample_data()` and the call to `add_datapoint()` in the
main loop with your own feed.  The plotting code is entirely decoupled from
the data source — all it needs is:

```c
add_datapoint(&ctx.series, your_float_value);
```

### Customising labels

Set these fields after `init_timeseries()` and before the event loop:

```c
strncpy(ctx.x_label,     "Time (ms)",   sizeof(ctx.x_label)     - 1);
strncpy(ctx.y_label,     "Voltage (V)", sizeof(ctx.y_label)     - 1);
strncpy(ctx.series_name, "ADC Ch0",     sizeof(ctx.series_name) - 1);
```

### Changing the buffer size

Adjust `MAX_POINTS` at the top of `sdlplot.cpp`.  Larger values consume more
memory but allow zooming out further.

---

## Constants reference

| Constant | Default | Effect |
|---|---|---|
| `MAX_POINTS` | 400 | Ring-buffer capacity (max samples retained) |
| `MARGIN_L/T/R/B` | 65/30/20/70 | Pixel margins around the plot area |
| `MIN_X_VISIBLE` | 10 | Minimum samples shown when X-zoomed in |
| `ZOOM_IN_FAC` | 0.80 | Multiplier applied to visible window on zoom-in |
| `ZOOM_OUT_FAC` | 1.25 | Multiplier applied to visible window on zoom-out |
| `X_GRID_DIVS` | 6 | Target number of vertical grid divisions |
| `Y_GRID_DIVS` | 5 | Target number of horizontal grid divisions |

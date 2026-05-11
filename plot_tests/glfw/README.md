# glfw_plot — Real-time Multi-series Time Series Plotter (GLFW + OpenGL)

A lightweight real-time plotter built on **GLFW** and **legacy OpenGL** (immediate
mode).  Plots multiple simultaneous signals in a scrolling time window with a
moving grid, Y auto-scaling, pause/resume, and keyboard controls — no extra
dependencies beyond GLFW and the system OpenGL.

---

## Features

| Feature | Detail |
|---|---|
| Multiple series | Three simultaneous signals, each with its own colour and line width |
| Scrolling time window | Fixed-width time window tracks the latest data; old data is dropped |
| Moving grid | Grid lines are computed in data-space and scroll with the signals |
| Major / minor grid lines | Every 5th vertical line and the Y=0 horizontal line are drawn brighter |
| Y auto-scale | Rescales the Y axis every frame to fit visible data, with 10 % padding |
| Pause / Resume | `Space` freezes the simulation; data stops but the window is still interactive |
| Reset | `R` clears all series and restarts from t = 0 |
| Time-window zoom | `+` / `-` shrinks or widens the visible time window (1 s … 60 s) |
| Auto-scale toggle | `A` switches between auto Y and a fixed ±2 range |
| VSync + anti-aliasing | `glfwSwapInterval(1)` + `GL_LINE_SMOOTH` + blend |
| Resizable window | Framebuffer size callback keeps the viewport correct |

---

## Architecture

```
PlotConfig
├── xMin, xMax          current visible time range (updated every frame)
├── yMin, yMax          current Y range (auto or manual)
├── windowWidth         visible time span in seconds
└── autoScale           flag to enable/disable Y auto-scaling

TimeSeries              std::deque<float> timeData + valueData
├── r, g, b             line colour
├── lineWidth
└── maxPoints           deque is trimmed to this length when points are added
```

### Data flow (per frame)

```
glfwGetTime() / deltaTime
      │
addPoint()               push (currentTime, value) into each deque, trim front
      │
updatePlotConfig()       advance xMin/xMax with the window; Y auto-scale over
                         visible points
      │
drawGrid()               vertical lines every calculateNiceGridSpacing(xRange,10)
                         horizontal lines every calculateNiceGridSpacing(yRange,8)
                         major lines at every 5th interval (brighter shade)
drawAxes()               X axis at y=0 and Y axis at x=0 (if in range)
plotTimeSeries()         GL_LINE_STRIP, clipped to [xMin,xMax], mapped to [-1,1]
                         then corrected for aspect ratio via normalizedToScreen()
```

### Coordinate system

All drawing is done in the GL clip space `[-1, 1] × [-1, 1]`.  Because
`glOrtho` is not called, a non-square window would otherwise distort the plot.
`normalizedToScreen()` compensates:

```
aspect = width / height

if aspect >= 1  →  xScreen = xNorm / aspect,  yScreen = yNorm
else            →  xScreen = xNorm,            yScreen = yNorm * aspect
```

This keeps the grid square regardless of window shape.

### Grid spacing

`calculateNiceGridSpacing(range, targetDivisions)` uses the same 1/2/5 ×
power-of-10 algorithm as the SDL3 version:

```
roughSpacing = range / targetDivisions
magnitude    = 10 ^ floor(log10(roughSpacing))
normalized   = roughSpacing / magnitude

≤1.5 → 1×mag  |  ≤3.0 → 2×mag  |  ≤7.0 → 5×mag  |  else → 10×mag
```

### Signal generators

| Series | Formula | Colour |
|---|---|---|
| signal1 | `sin(2π·1·t)` | Green-ish (0.2, 1.0, 0.5) |
| signal2 | `cos(2π·2.5·t) · (0.5 + 0.5·sin(0.5t))` — AM-modulated | Orange |
| signal3 | `0.5·(sin(2π·0.5·t) + sin(2π·0.6·t))` — beat pattern | Blue |

---

## Build

### Manual

```bash
g++ -o glfw_plot glfw_plot.cpp -lglfw -lGL
./glfw_plot
```

### CMake (this directory)

```bash
mkdir build && cd build
cmake ..
cmake --build .
./glfw_plot
```

### CMake (parent plot_tests project)

```bash
cd plot_tests
mkdir build && cd build
cmake ..
cmake --build . --target glfw_plot
```

---

## Controls

| Key | Action |
|---|---|
| `Esc` | Exit |
| `Space` | Pause / Resume |
| `R` | Reset all data and time |
| `+` or numpad `+` | Decrease time window width (-0.1 s, min 1 s) |
| `-` or numpad `-` | Increase time window width (+0.1 s, max 60 s) |
| `A` | Toggle Y auto-scaling (off → fixed ±2) |

Window title bar shows current simulation time, window width, and pause state.

---

## Integrating your own data source

Replace the three signal generators and `addPoint` calls in the main loop:

```cpp
// your_value comes from a sensor, queue, shared memory, etc.
signal1.addPoint(currentTime, your_value);
```

To add more series:

```cpp
TimeSeries mySignal(r, g, b, lineWidth, maxPoints);
allSeries.push_back(&mySignal);
// then in the loop:
mySignal.addPoint(currentTime, value);
```

`updatePlotConfig` automatically picks up all series in `allSeries` for
auto-scaling.

---

## Constants / tunables

| Symbol | Default | Effect |
|---|---|---|
| `windowWidth` (PlotConfig) | `10.0f` | Initial visible time span (seconds) |
| `maxPoints` (TimeSeries) | `10000` | Maximum retained samples per series |
| `targetDivisions` X grid | `10` | Approximate number of vertical grid lines |
| `targetDivisions` Y grid | `8` | Approximate number of horizontal grid lines |
| major-line interval | every 5th | `x % (xSpacing*5) < xSpacing*0.01` |
| Y padding | 10 % | `range * 0.1f` added above and below visible data |
| min/max window | 1 s / 60 s | Clamped in `+`/`-` key handlers |

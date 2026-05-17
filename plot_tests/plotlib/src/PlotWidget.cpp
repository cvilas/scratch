#include "plotlib/PlotWidget.h"
#include "embedded_font.h"

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <memory>
#include <span>
#include <string_view>
#include <unordered_map>
#include <vector>
#include <random>

// ── Layout / style constants ──────────────────────────────────────────────────
static constexpr int   MARGIN_L    = 70;
static constexpr int   MARGIN_T    = 36;
static constexpr int   MARGIN_R    = 20;
static constexpr int   MARGIN_B    = 72;
static constexpr int   TICK_LEN    = 6;
static constexpr int   X_GRID_DIVS = 6;
static constexpr int   Y_GRID_DIVS = 5;
static constexpr float ZOOM_IN     = 0.80f;
static constexpr int   FONT_PT     = 20;
static constexpr int   POINT_R     = 4;  // symbol radius


//-------------------------------------------------------------------------------------------------
static auto generateRandomColor() -> SDL_Color {
  /// Create a random pastel colour
  /// Reference:
  /// https://martin.ankerl.com/2009/12/09/how-to-create-random-colors-programmatically/

  constexpr auto GOLDEN_RATIO_CONJUGATE = 0.618033988749895;

  static std::random_device rd;
  static std::mt19937 mt(rd());
  static std::uniform_real_distribution<double> dist(0., 1.);

  auto hue = std::fmod(dist(mt) + GOLDEN_RATIO_CONJUGATE, 1.);

  // HSV → RGB  (S = 0.5, V = 0.95 for a pastel palette)
  constexpr double s = 0.5, v = 0.95; // NOLINT(cppcoreguidelines-avoid-magic-numbers)
  double h6 = hue * 6.0;
  int    i  = static_cast<int>(h6);
  double f  = h6 - i;
  double p  = v * (1.0 - s);
  double q  = v * (1.0 - s * f);
  double t  = v * (1.0 - s * (1.0 - f));
  double r{}, g{}, b{};
  switch (i % 6) {
  case 0: r = v; g = t; b = p; break;
  case 1: r = q; g = v; b = p; break;
  case 2: r = p; g = v; b = t; break;
  case 3: r = p; g = q; b = v; break;
  case 4: r = t; g = p; b = v; break;
  default: r = v; g = p; b = q; break;
  }
  return {static_cast<uint8_t>(r * 255),
          static_cast<uint8_t>(g * 255),
          static_cast<uint8_t>(b * 255),
          255};
}

// ── Nice-step helper ──────────────────────────────────────────────────────────
static double niceStep(double range, int n) {
    if (range <= 0.0 || n <= 0) return 1.0;
    double rough = range / n;
    double mag   = std::pow(10.0, std::floor(std::log10(rough)));
    double norm  = rough / mag;
    double k     = (norm < 1.5) ? 1.0 : (norm < 3.5) ? 2.0 : (norm < 7.5) ? 5.0 : 10.0;
    return k * mag;
}

// ── Pre-computed point-symbol offsets (POINT_R = 4) ─────────────────────────
// Computed at compile time via consteval — zero runtime initialisation cost.
namespace {
    // Integer nearest-sqrt: std::sqrt is not constexpr until C++26.
    consteval int iRoundSqrt(int n) noexcept {
        int r = 0;
        while ((r + 1) * (r + 1) <= n) ++r;
        return ((r + 1) * (r + 1) - n < n - r * r) ? r + 1 : r;
    }
    consteval std::size_t kDiscCount() noexcept {
        std::size_t n = 0;
        for (int i = -POINT_R; i <= POINT_R; ++i)
            for (int j = -POINT_R; j <= POINT_R; ++j)
                if (i*i + j*j <= POINT_R*POINT_R) ++n;
        return n;
    }
    consteval std::size_t kCircleCount() noexcept {
        std::size_t n = 0;
        for (int i = -POINT_R; i <= POINT_R; ++i) {
            const int j = iRoundSqrt(POINT_R*POINT_R - i*i);
            n += (j == 0) ? 1 : 2;
        }
        return n;
    }
    constexpr auto k_discOffsets = []() consteval {
        std::array<SDL_FPoint, kDiscCount()> arr{};
        std::size_t k = 0;
        for (int i = -POINT_R; i <= POINT_R; ++i)
            for (int j = -POINT_R; j <= POINT_R; ++j)
                if (i*i + j*j <= POINT_R*POINT_R)
                    arr[k++] = {static_cast<float>(i), static_cast<float>(j)};
        return arr;
    }();
    constexpr auto k_circleOffsets = []() consteval {
        std::array<SDL_FPoint, kCircleCount()> arr{};
        std::size_t k = 0;
        for (int i = -POINT_R; i <= POINT_R; ++i) {
            const int j = iRoundSqrt(POINT_R*POINT_R - i*i);
            arr[k++] = {static_cast<float>(i), static_cast<float>(j)};
            if (j != 0) arr[k++] = {static_cast<float>(i), static_cast<float>(-j)};
        }
        return arr;
    }();
} // namespace

// ── SDL reference counting ────────────────────────────────────────────────────
static int s_sdlRefs = 0;
static int s_ttfRefs = 0;

// ── Private implementation ────────────────────────────────────────────────────
struct PlotWidget::Private {
    SDL_Window   *window   = nullptr;
    SDL_Renderer *renderer = nullptr;
    TTF_Font     *font     = nullptr;
    SDL_WindowID  windowId = 0;
    bool          open     = true;

    std::string title;
    std::string xLabel{"X"};
    std::string yLabel{"Y"};
    bool        legendEnabled = true;

    std::vector<std::unique_ptr<Trace>> traces;

    // ── Text-texture cache ────────────────────────────────────────────────
    // Textures are rendered once as white and colour-modulated at draw time.
    struct TextureEntry { SDL_Texture *tex = nullptr; float w = 0.f, h = 0.f; };
    // Transparent hash/compare: find() accepts std::string_view / const char*
    // without constructing a temporary std::string on each cache lookup.
    struct StringViewHash {
        using is_transparent = void;
        [[nodiscard]] std::size_t operator()(std::string_view sv) const noexcept {
            return std::hash<std::string_view>{}(sv);
        }
    };
    mutable std::unordered_map<std::string, TextureEntry,
                               StringViewHash, std::equal_to<>> textCache;

    // ── Legend name-width cache ───────────────────────────────────────────
    size_t cachedLegendTraceCount = 0;
    int    cachedLegendNameW      = 0;

    // ── Layout ────────────────────────────────────────────────────────────
    int plotX{}, plotY{}, plotW{}, plotH{};

    // ── View state ────────────────────────────────────────────────────────
    double xRangeWidth = 0.0;  // 0 = show all; >0 = show last N x-units
    double xViewMin = 0.0, xViewMax = 1.0;
    double vyMin = -1.0, vyMax = 1.0;
    bool   yManual = false;

    // ── Zoom history (undo stack for scroll-wheel zoom-in) ────────────────
    struct ZoomState { double xRangeWidth, vyMin, vyMax; bool yManual; };
    std::vector<ZoomState> zoomHistory;

    // ── Grid step cache (computed once per frame in updateView) ───────────
    double xGridStep = 1.0, xGridFirst = 0.0;
    double yGridStep = 1.0, yGridFirst = 0.0;

    // ── Legend ────────────────────────────────────────────────────────────
    // normX/normY are the legend's top-left corner as fractions of the window
    // size.  Pixel x/y are derived from them in recalcLayout() so the legend
    // keeps its relative position when the window is resized.
    struct Legend {
        float x{}, y{}, w{}, h{};
        float normX{0.85f}, normY{0.08f};  // default: top-right
        bool  dragging{};
        float dragDx{}, dragDy{};
    } legend{};

    // ── Y-axis pan state ──────────────────────────────────────────────────
    bool  panDragging = false;
    float panLastY    = 0.0f;

    // ── Helpers ───────────────────────────────────────────────────────────
    void recalcLayout();
    void updateView();

    void drawGrid()       const;
    void drawAxes()       const;
    void drawTraces()     const;
    void drawAxisLabels() const;
    void drawLegend();
    void drawTitle()      const;

    const TextureEntry *cachedText(const char *text) const;
    void renderText(const char *text, float x, float y, SDL_Color col) const;
    void renderTextVertical(const char *text, float cx, float cy, SDL_Color col) const;
    void drawPointSymbol(Trace::PointStyle ps, float sx, float sy, SDL_Color col) const;

    float  dataToScreenX(double x) const;
    float  dataToScreenY(double y) const;
    double screenToDataY(float sy) const;
    bool   inPlot(float mx, float my) const;
    bool   legendHit(float mx, float my) const;
};

// ── Layout ────────────────────────────────────────────────────────────────────
void PlotWidget::Private::recalcLayout() {
    int w, h;
    SDL_GetWindowSize(window, &w, &h);
    plotX = MARGIN_L;
    plotY = MARGIN_T;
    plotW = w - MARGIN_L - MARGIN_R;
    plotH = h - MARGIN_T - MARGIN_B;
    // Restore legend pixel position from its normalized (fractional) coords
    legend.x = legend.normX * static_cast<float>(w);
    legend.y = legend.normY * static_cast<float>(h);
}

// ── View update ───────────────────────────────────────────────────────────────
void PlotWidget::Private::updateView() {
    // Find full data extent
    bool   hasData = false;
    double dxMin = 0.0, dxMax = 1.0;

    for (const auto &tr : traces) {
        if (tr->m_x.empty()) continue;
        if (!hasData) {
            dxMin = dxMax = tr->m_x[0];
            hasData = true;
        }
        auto [s1, s2] = tr->m_x.spans();
        for (double v : s1) { dxMin = std::min(dxMin, v); dxMax = std::max(dxMax, v); }
        for (double v : s2) { dxMin = std::min(dxMin, v); dxMax = std::max(dxMax, v); }
    }

    // X view
    if (xRangeWidth > 0.0) {
        xViewMax = dxMax;
        xViewMin = dxMax - xRangeWidth;
    } else {
        xViewMin = dxMin;
        xViewMax = dxMax;
    }
    if (xViewMax - xViewMin < 1e-12) { xViewMin -= 1.0; xViewMax += 1.0; }

    // Y view (auto-scale from visible points)
    if (!yManual) {
        bool first = true;
        for (const auto &tr : traces) {
            auto [xs1, xs2] = tr->m_x.spans();
            auto [ys1, ys2] = tr->m_y.spans();
            for (size_t i = 0; i < xs1.size(); ++i) {
                if (xs1[i] < xViewMin || xs1[i] > xViewMax) continue;
                if (first) { vyMin = vyMax = ys1[i]; first = false; }
                else       { vyMin = std::min(vyMin, ys1[i]); vyMax = std::max(vyMax, ys1[i]); }
            }
            for (size_t i = 0; i < xs2.size(); ++i) {
                if (xs2[i] < xViewMin || xs2[i] > xViewMax) continue;
                if (first) { vyMin = vyMax = ys2[i]; first = false; }
                else       { vyMin = std::min(vyMin, ys2[i]); vyMax = std::max(vyMax, ys2[i]); }
            }
        }
        if (first && hasData) {
            // No points in X window – fall back to global Y extent
            for (const auto &tr : traces) {
                auto [s1, s2] = tr->m_y.spans();
                for (double v : s1) {
                    if (first) { vyMin = vyMax = v; first = false; }
                    else       { vyMin = std::min(vyMin, v); vyMax = std::max(vyMax, v); }
                }
                for (double v : s2) {
                    if (first) { vyMin = vyMax = v; first = false; }
                    else       { vyMin = std::min(vyMin, v); vyMax = std::max(vyMax, v); }
                }
            }
        }
        // 5% padding
        double pad = (vyMax - vyMin) * 0.05;
        vyMin -= pad; vyMax += pad;
    }
    if (vyMax - vyMin < 1e-12) { vyMin -= 1.0; vyMax += 1.0; }

    // Cache grid step values shared by drawGrid() and drawAxisLabels()
    xGridStep  = niceStep(xViewMax - xViewMin, X_GRID_DIVS);
    xGridFirst = std::ceil(xViewMin / xGridStep) * xGridStep;
    yGridStep  = niceStep(vyMax - vyMin, Y_GRID_DIVS);
    yGridFirst = std::ceil(vyMin / yGridStep) * yGridStep;
}

// ── Coordinate helpers ────────────────────────────────────────────────────────
float PlotWidget::Private::dataToScreenX(double x) const {
    return static_cast<float>(plotX + (x - xViewMin) / (xViewMax - xViewMin) * plotW);
}
float PlotWidget::Private::dataToScreenY(double y) const {
    return static_cast<float>(plotY + plotH - (y - vyMin) / (vyMax - vyMin) * plotH);
}
double PlotWidget::Private::screenToDataY(float sy) const {
    const double norm = 1.0 - static_cast<double>(sy - plotY) / plotH;
    return vyMin + norm * (vyMax - vyMin);
}
bool PlotWidget::Private::inPlot(float mx, float my) const {
    return mx >= plotX && mx <= plotX + plotW && my >= plotY && my <= plotY + plotH;
}
bool PlotWidget::Private::legendHit(float mx, float my) const {
    return mx >= legend.x && mx <= legend.x + legend.w &&
           my >= legend.y && my <= legend.y + legend.h;
}

// ── Text rendering ────────────────────────────────────────────────────────────
const PlotWidget::Private::TextureEntry *
PlotWidget::Private::cachedText(const char *text) const {
    auto it = textCache.find(text);
    if (it == textCache.end()) [[unlikely]] {
        const std::unique_ptr<SDL_Surface, decltype(&SDL_DestroySurface)> surf{
            TTF_RenderText_Blended(font, text, 0, {255, 255, 255, 255}),
            SDL_DestroySurface
        };
        if (!surf) return nullptr;
        SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer, surf.get());
        if (!tex) return nullptr;
        const float tw = static_cast<float>(surf->w), th = static_cast<float>(surf->h);
        it = textCache.emplace(text, TextureEntry{tex, tw, th}).first;
    }
    return &it->second;
}

void PlotWidget::Private::renderText(const char *text, float x, float y,
                                     SDL_Color col) const {
    const auto *e = cachedText(text);
    if (!e) return;
    SDL_SetTextureColorMod(e->tex, col.r, col.g, col.b);
    SDL_FRect dst = {x, y, e->w, e->h};
    SDL_RenderTexture(renderer, e->tex, nullptr, &dst);
}

void PlotWidget::Private::renderTextVertical(const char *text,
                                              float cx, float cy,
                                              SDL_Color col) const {
    const auto *e = cachedText(text);
    if (!e) return;
    SDL_SetTextureColorMod(e->tex, col.r, col.g, col.b);
    // Place at natural size centred on (cx, cy), rotate -90° around that centre
    SDL_FRect dst = {cx - e->w / 2.0f, cy - e->h / 2.0f, e->w, e->h};
    SDL_RenderTextureRotated(renderer, e->tex, nullptr, &dst, -90.0, nullptr, SDL_FLIP_NONE);
}

// ── Point symbol ──────────────────────────────────────────────────────────────
void PlotWidget::Private::drawPointSymbol(Trace::PointStyle ps,
                                           float sx, float sy,
                                           SDL_Color col) const {
    if (ps == Trace::PointStyle::None) return;
    SDL_SetRenderDrawColor(renderer, col.r, col.g, col.b, col.a);
    const float R = static_cast<float>(POINT_R);

    // Render a pre-computed offset list translated to (sx, sy)
    auto renderPts = [&](std::span<const SDL_FPoint> offs) {
        std::array<SDL_FPoint, 64> buf; // POINT_R=4: disc=49, circle=16
        for (std::size_t k = 0; k < offs.size(); ++k)
            buf[k] = {sx + offs[k].x, sy + offs[k].y};
        SDL_RenderPoints(renderer, buf.data(), static_cast<int>(offs.size()));
    };

    switch (ps) {
    case Trace::PointStyle::None: break; // unreachable; guarded above

    case Trace::PointStyle::Dot: {
        static constexpr SDL_FPoint dotOff[] = {
            {-1,-1},{0,-1},{1,-1},{-1,0},{0,0},{1,0},{-1,1},{0,1},{1,1}
        };
        SDL_FPoint buf[9];
        for (int k = 0; k < 9; ++k) buf[k] = {sx + dotOff[k].x, sy + dotOff[k].y};
        SDL_RenderPoints(renderer, buf, 9);
        break;
    }

    case Trace::PointStyle::Disc:
        renderPts(k_discOffsets);
        break;

    case Trace::PointStyle::Circle:
        renderPts(k_circleOffsets);
        break;

    case Trace::PointStyle::Cross:
        SDL_RenderLine(renderer, sx - R, sy - R, sx + R, sy + R);
        SDL_RenderLine(renderer, sx + R, sy - R, sx - R, sy + R);
        break;

    case Trace::PointStyle::Plus:
        SDL_RenderLine(renderer, sx - R, sy, sx + R, sy);
        SDL_RenderLine(renderer, sx, sy - R, sx, sy + R);
        break;

    case Trace::PointStyle::Square:
        SDL_RenderLine(renderer, sx-R, sy-R, sx+R, sy-R);
        SDL_RenderLine(renderer, sx+R, sy-R, sx+R, sy+R);
        SDL_RenderLine(renderer, sx+R, sy+R, sx-R, sy+R);
        SDL_RenderLine(renderer, sx-R, sy+R, sx-R, sy-R);
        break;

    case Trace::PointStyle::Diamond:
        SDL_RenderLine(renderer, sx,   sy-R, sx+R, sy  );
        SDL_RenderLine(renderer, sx+R, sy,   sx,   sy+R);
        SDL_RenderLine(renderer, sx,   sy+R, sx-R, sy  );
        SDL_RenderLine(renderer, sx-R, sy,   sx,   sy-R);
        break;

    case Trace::PointStyle::Triangle:
        SDL_RenderLine(renderer, sx,   sy-R, sx+R, sy+R);
        SDL_RenderLine(renderer, sx+R, sy+R, sx-R, sy+R);
        SDL_RenderLine(renderer, sx-R, sy+R, sx,   sy-R);
        break;

    case Trace::PointStyle::TriangleInverted:
        SDL_RenderLine(renderer, sx,   sy+R, sx+R, sy-R);
        SDL_RenderLine(renderer, sx+R, sy-R, sx-R, sy-R);
        SDL_RenderLine(renderer, sx-R, sy-R, sx,   sy+R);
        break;

    case Trace::PointStyle::Star: {
        float h = R * 0.866f, r2 = R * 0.5f;
        SDL_RenderLine(renderer, sx,   sy-R, sx+h, sy+r2);
        SDL_RenderLine(renderer, sx+h, sy+r2, sx-h, sy+r2);
        SDL_RenderLine(renderer, sx-h, sy+r2, sx,   sy-R );
        SDL_RenderLine(renderer, sx,   sy+R, sx+h, sy-r2);
        SDL_RenderLine(renderer, sx+h, sy-r2, sx-h, sy-r2);
        SDL_RenderLine(renderer, sx-h, sy-r2, sx,   sy+R );
        break;
    }

    case Trace::PointStyle::CrossSquare:
        SDL_RenderLine(renderer, sx-R, sy-R, sx+R, sy-R);
        SDL_RenderLine(renderer, sx+R, sy-R, sx+R, sy+R);
        SDL_RenderLine(renderer, sx+R, sy+R, sx-R, sy+R);
        SDL_RenderLine(renderer, sx-R, sy+R, sx-R, sy-R);
        SDL_RenderLine(renderer, sx-R, sy-R, sx+R, sy+R);
        SDL_RenderLine(renderer, sx+R, sy-R, sx-R, sy+R);
        break;

    case Trace::PointStyle::PlusSquare:
        SDL_RenderLine(renderer, sx-R, sy-R, sx+R, sy-R);
        SDL_RenderLine(renderer, sx+R, sy-R, sx+R, sy+R);
        SDL_RenderLine(renderer, sx+R, sy+R, sx-R, sy+R);
        SDL_RenderLine(renderer, sx-R, sy+R, sx-R, sy-R);
        SDL_RenderLine(renderer, sx-R, sy,   sx+R, sy  );
        SDL_RenderLine(renderer, sx,   sy-R, sx,   sy+R);
        break;

    case Trace::PointStyle::CrossCircle:
        renderPts(k_circleOffsets);
        SDL_RenderLine(renderer, sx-R, sy-R, sx+R, sy+R);
        SDL_RenderLine(renderer, sx+R, sy-R, sx-R, sy+R);
        break;

    case Trace::PointStyle::PlusCircle:
        renderPts(k_circleOffsets);
        SDL_RenderLine(renderer, sx-R, sy,   sx+R, sy  );
        SDL_RenderLine(renderer, sx,   sy-R, sx,   sy+R);
        break;

    case Trace::PointStyle::Peace:
        renderPts(k_circleOffsets);
        SDL_RenderLine(renderer, sx, sy-R, sx, sy+R);
        SDL_RenderLine(renderer, sx, sy, sx - R*0.707f, sy + R*0.707f);
        SDL_RenderLine(renderer, sx, sy, sx + R*0.707f, sy + R*0.707f);
        break;
    }
}

// ── Grid ──────────────────────────────────────────────────────────────────────
void PlotWidget::Private::drawGrid() const {
    SDL_SetRenderDrawColor(renderer, 48, 48, 48, 255);
    for (double gx = xGridFirst; gx <= xViewMax + xGridStep * 0.01; gx += xGridStep) {
        const float sx = dataToScreenX(gx);
        if (sx < plotX || sx > plotX + plotW) continue;
        SDL_RenderLine(renderer, sx, static_cast<float>(plotY), sx, static_cast<float>(plotY + plotH));
    }
    for (double gy = yGridFirst; gy <= vyMax + yGridStep * 0.01; gy += yGridStep) {
        const float sy = dataToScreenY(gy);
        if (sy < plotY || sy > plotY + plotH) continue;
        SDL_RenderLine(renderer, static_cast<float>(plotX), sy, static_cast<float>(plotX + plotW), sy);
    }
}

// ── Axes ──────────────────────────────────────────────────────────────────────
void PlotWidget::Private::drawAxes() const {
    SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
    SDL_RenderLine(renderer, static_cast<float>(plotX), static_cast<float>(plotY),
                   static_cast<float>(plotX), static_cast<float>(plotY + plotH));
    SDL_RenderLine(renderer, static_cast<float>(plotX), static_cast<float>(plotY + plotH),
                   static_cast<float>(plotX + plotW), static_cast<float>(plotY + plotH));
}

// ── Traces ────────────────────────────────────────────────────────────────────
void PlotWidget::Private::drawTraces() const {
    std::vector<SDL_FPoint> pts;      // reused across traces
    std::vector<SDL_FPoint> stepPts;  // reused for Step-style expansion
    for (const auto &tr : traces) {
        const auto &xs = tr->m_x;
        const auto &ys = tr->m_y;
        if (xs.empty()) continue;

        SDL_Color col = {tr->m_r, tr->m_g, tr->m_b, tr->m_a};
        SDL_SetRenderDrawColor(renderer, col.r, col.g, col.b, col.a);

        pts.clear();
        pts.reserve(xs.size());
        double xPad = (xViewMax - xViewMin) * 0.005;
        auto addPts = [&](std::span<const double> xsp, std::span<const double> ysp) {
            for (size_t i = 0; i < xsp.size(); ++i) {
                if (xsp[i] < xViewMin - xPad || xsp[i] > xViewMax + xPad) continue;
                pts.push_back({dataToScreenX(xsp[i]), dataToScreenY(ysp[i])});
            }
        };
        auto [xs1, xs2] = xs.spans();
        auto [ys1, ys2] = ys.spans();
        addPts(xs1, ys1);
        addPts(xs2, ys2);
        if (pts.empty()) continue;

        switch (tr->m_lineStyle) {
        case Trace::LineStyle::Line:
            SDL_RenderLines(renderer, pts.data(), static_cast<int>(pts.size()));
            break;

        case Trace::LineStyle::Step: {
            stepPts.clear();
            stepPts.reserve(pts.size() * 2 - 1);
            stepPts.push_back(pts[0]);
            for (std::size_t i = 1; i < pts.size(); ++i) {
                stepPts.push_back({pts[i].x, pts[i-1].y});
                stepPts.push_back(pts[i]);
            }
            SDL_RenderLines(renderer, stepPts.data(), static_cast<int>(stepPts.size()));
            break;
        }

        case Trace::LineStyle::Lollipop: {
            const float base = std::clamp(dataToScreenY(0.0),
                                          static_cast<float>(plotY),
                                          static_cast<float>(plotY + plotH));
            for (const auto &p : pts)
                SDL_RenderLine(renderer, p.x, base, p.x, p.y);
            break;
        }

        case Trace::LineStyle::None:
            break;
        }

        if (tr->m_pointStyle != Trace::PointStyle::None)
            for (const auto &p : pts)
                drawPointSymbol(tr->m_pointStyle, p.x, p.y, col);
    }
}

// ── Axis labels & ticks ───────────────────────────────────────────────────────
void PlotWidget::Private::drawAxisLabels() const {
    int fh = TTF_GetFontHeight(font);

    // Y ticks
    SDL_SetRenderDrawColor(renderer, 180, 180, 180, 255);
    for (double gy = yGridFirst; gy <= vyMax + yGridStep * 0.01; gy += yGridStep) {
        const float norm = static_cast<float>((gy - vyMin) / (vyMax - vyMin));
        if (norm < 0.0f || norm > 1.0f) continue;
        const float sy = static_cast<float>(plotY + plotH) - norm * static_cast<float>(plotH);
        SDL_RenderLine(renderer, static_cast<float>(plotX - TICK_LEN), sy, static_cast<float>(plotX), sy);
        char lbl[32];
        std::snprintf(lbl, sizeof lbl, "%.4g", gy);
        int tw = 0, th = 0;
        TTF_GetStringSize(font, lbl, 0, &tw, &th);
        renderText(lbl, static_cast<float>(plotX - TICK_LEN) - tw - 2, sy - th / 2.0f, {200, 200, 200, 255});
    }

    if (!yLabel.empty())
        renderTextVertical(yLabel.c_str(), fh / 2.0f + 2,
                           static_cast<float>(plotY + plotH / 2), {160, 160, 255, 255});

    // X ticks
    const float baseY = static_cast<float>(plotY + plotH);
    SDL_SetRenderDrawColor(renderer, 180, 180, 180, 255);
    for (double gx = xGridFirst; gx <= xViewMax + xGridStep * 0.01; gx += xGridStep) {
        const float sx = dataToScreenX(gx);
        if (sx < plotX || sx > plotX + plotW) continue;
        SDL_RenderLine(renderer, sx, baseY, sx, baseY + TICK_LEN);
        char lbl[32];
        std::snprintf(lbl, sizeof lbl, "%.4g", gx);
        int tw = 0, th = 0;
        TTF_GetStringSize(font, lbl, 0, &tw, &th);
        renderText(lbl, sx - tw / 2.0f, baseY + TICK_LEN + 2, {200, 200, 200, 255});
    }

    if (!xLabel.empty()) {
        int tw = 0, th = 0;
        TTF_GetStringSize(font, xLabel.c_str(), 0, &tw, &th);
        renderText(xLabel.c_str(), static_cast<float>(plotX) + plotW / 2.0f - tw / 2.0f,
                   baseY + TICK_LEN + fh + 4, {160, 160, 255, 255});
    }

    if (xRangeWidth > 0.0 || yManual)
        renderText("dbl-click: reset zoom",
                   static_cast<float>(plotX) + 4, static_cast<float>(plotY) + 4,
                   {100, 100, 100, 255});
}

// ── Legend ────────────────────────────────────────────────────────────────────
void PlotWidget::Private::drawLegend() {
    if (!legendEnabled || traces.empty()) return;

    int fh   = TTF_GetFontHeight(font);
    int rowH = fh + 8;

    // Recompute max name width only when a new trace is added
    if (traces.size() != cachedLegendTraceCount) {
        cachedLegendNameW = 0;
        for (const auto &tr : traces) {
            int tw = 0, th = 0;
            TTF_GetStringSize(font, tr->m_name.c_str(), 0, &tw, &th);
            cachedLegendNameW = std::max(cachedLegendNameW, tw);
        }
        cachedLegendTraceCount = traces.size();
    }
    legend.w = static_cast<float>(28 + cachedLegendNameW + 8);
    legend.h = static_cast<float>(rowH * static_cast<int>(traces.size()) + 6);

    SDL_FRect bg = {legend.x, legend.y, legend.w, legend.h};
    SDL_SetRenderDrawColor(renderer, 18, 18, 18, 215);
    SDL_RenderFillRect(renderer, &bg);
    SDL_SetRenderDrawColor(renderer, 160, 160, 160, 255);
    SDL_RenderRect(renderer, &bg);

    for (size_t i = 0; i < traces.size(); ++i) {
        const auto &tr  = traces[i];
        const float ry  = legend.y + 3.0f + static_cast<float>(i) * rowH;
        float        mid = ry + rowH / 2.0f;

        SDL_SetRenderDrawColor(renderer, tr->m_r, tr->m_g, tr->m_b, tr->m_a);
        SDL_RenderLine(renderer, legend.x + 4, mid, legend.x + 20, mid);

        SDL_Color col = {230, 230, 230, 255};
        renderText(tr->m_name.c_str(),
                   legend.x + 24,
                   ry + (rowH - fh) / 2.0f, col);
    }
}

// ── Title ─────────────────────────────────────────────────────────────────────
void PlotWidget::Private::drawTitle() const {
    if (title.empty()) return;
    int tw = 0, th = 0;
    TTF_GetStringSize(font, title.c_str(), 0, &tw, &th);
    SDL_Color col = {240, 240, 240, 255};
    renderText(title.c_str(),
               static_cast<float>(plotX) + plotW / 2.0f - tw / 2.0f,
               static_cast<float>(MARGIN_T - th) / 2.0f, col);
}

// ────────────────────────────────────────────────────────────────────────────
// PlotWidget public interface
// ────────────────────────────────────────────────────────────────────────────
PlotWidget::PlotWidget(int width, int height, const std::string &title)
    : d(std::make_unique<Private>()) {
    d->title = title;

    if (++s_sdlRefs == 1)
        SDL_Init(SDL_INIT_VIDEO);
    if (++s_ttfRefs == 1)
        TTF_Init();

    d->window = SDL_CreateWindow(title.c_str(), width, height,
                                  SDL_WINDOW_RESIZABLE);
    d->renderer  = SDL_CreateRenderer(d->window, nullptr);
    // Try adaptive vsync first (smooth on slow frames), fall back to regular vsync.
    if (!SDL_SetRenderVSync(d->renderer, SDL_RENDERER_VSYNC_ADAPTIVE))
        if (!SDL_SetRenderVSync(d->renderer, 1))
            SDL_Log("plotlib: vsync unavailable – tearing may occur (%s)", SDL_GetError());
    d->windowId  = SDL_GetWindowID(d->window);

    // Load the font compiled into the binary via C++23 #embed.
    // SDL_IOFromConstMem wraps the array without copying; TTF_OpenFontIO
    // reads it as if it were a file (SDL_ttf/FreeType supports OTF natively).
    {
        auto          bytes = embeddedFontData();
        SDL_IOStream *io    = SDL_IOFromConstMem(bytes.data(), bytes.size());
        if (io)
            d->font = TTF_OpenFontIO(io, /*closeio=*/true, FONT_PT);
    }

    // Legend default: top-right of plot area.  Compute normX/normY from
    // the intended pixel position so recalcLayout() can restore them later.
    {
        int w, h;
        SDL_GetWindowSize(d->window, &w, &h);
        d->legend.normX = static_cast<float>(width  - MARGIN_R - 130) / static_cast<float>(w);
        d->legend.normY = static_cast<float>(MARGIN_T + 10)            / static_cast<float>(h);
    }
    d->recalcLayout();
}

PlotWidget::~PlotWidget() {
    for (auto &[key, e] : d->textCache)
        SDL_DestroyTexture(e.tex);
    d->textCache.clear();
    if (d->font)     TTF_CloseFont(d->font);
    if (d->renderer) SDL_DestroyRenderer(d->renderer);
    if (d->window)   SDL_DestroyWindow(d->window);

    if (--s_ttfRefs == 0) TTF_Quit();
    if (--s_sdlRefs == 0) SDL_Quit();
}

// ── Setters / getters ─────────────────────────────────────────────────────────
void PlotWidget::setTitleText(const std::string &t) {
    d->title = t;
    SDL_SetWindowTitle(d->window, t.c_str());
}
auto PlotWidget::getTitleText() const -> std::string { return d->title; }

void PlotWidget::setAxisText(AxisId id, const std::string &label) {
    if (id == AxisId::AxisX) d->xLabel = label;
    else                     d->yLabel = label;
}
auto PlotWidget::getAxisText(AxisId id) const -> std::string {
    return (id == AxisId::AxisX) ? d->xLabel : d->yLabel;
}

void PlotWidget::enableLegend(bool on) { d->legendEnabled = on; }
auto PlotWidget::isLegendEnabled() const -> bool { return d->legendEnabled; }

bool PlotWidget::isOpen() const { return d->open; }

// ── Trace factory ─────────────────────────────────────────────────────────────
auto PlotWidget::trace(const std::string &name) -> Trace & {
    for (auto &tr : d->traces)
        if (tr->m_name == name) return *tr;

    // Create new trace
    auto *raw = new Trace(name);  // private ctor; PlotWidget is friend
    const auto color = generateRandomColor();
    raw->m_r = color.r;
    raw->m_g = color.g;
    raw->m_b = color.b;
    d->traces.emplace_back(raw);
    return *d->traces.back();
}

// ── Event pump ────────────────────────────────────────────────────────────────
bool PlotWidget::processEvents() {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        switch (e.type) {
        case SDL_EVENT_QUIT:
            d->open = false;
            break;

        case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
            if (e.window.windowID == d->windowId)
                d->open = false;
            break;

        case SDL_EVENT_WINDOW_RESIZED:
            if (e.window.windowID == d->windowId)
                d->recalcLayout();
            break;

        case SDL_EVENT_MOUSE_BUTTON_DOWN:
            if (e.button.button == SDL_BUTTON_LEFT) {
                float mx = e.button.x, my = e.button.y;
                if (d->legendEnabled && d->legendHit(mx, my)) {
                    d->legend.dragging = true;
                    d->legend.dragDx   = mx - d->legend.x;
                    d->legend.dragDy   = my - d->legend.y;
                } else if (e.button.clicks == 2 && d->inPlot(mx, my)) {
                    d->xRangeWidth = 0.0;
                    d->yManual     = false;
                    d->zoomHistory.clear();
                } else if (d->inPlot(mx, my)) {
                    d->panDragging = true;
                    d->panLastY    = my;
                }
            }
            break;

        case SDL_EVENT_MOUSE_MOTION:
            if (d->legend.dragging) {
                d->legend.x = e.motion.x - d->legend.dragDx;
                d->legend.y = e.motion.y - d->legend.dragDy;
                // Keep normalised position in sync so resize preserves it
                int winW, winH;
                SDL_GetWindowSize(d->window, &winW, &winH);
                d->legend.normX = d->legend.x / static_cast<float>(winW);
                d->legend.normY = d->legend.y / static_cast<float>(winH);
            } else if (d->panDragging) {
                const float dy = e.motion.y - d->panLastY;
                if (dy != 0.0f) {
                    const double shift =
                        -static_cast<double>(dy) * (d->vyMax - d->vyMin) / d->plotH;
                    d->vyMin  += shift;
                    d->vyMax  += shift;
                    d->yManual = true;
                }
                d->panLastY = e.motion.y;
            }
            break;

        case SDL_EVENT_MOUSE_BUTTON_UP:
            if (e.button.button == SDL_BUTTON_LEFT) {
                d->legend.dragging = false;
                d->panDragging     = false;
            }
            break;

        case SDL_EVENT_MOUSE_WHEEL: {
            float mx = 0, my = 0;
            SDL_GetMouseState(&mx, &my);
            if (!d->inPlot(mx, my)) break;

            bool ctrl = (SDL_GetModState() & SDL_KMOD_CTRL) != 0;

            if (e.wheel.y > 0) {
                // Zoom in — snapshot current state first, then apply
                d->zoomHistory.push_back({d->xRangeWidth, d->vyMin, d->vyMax, d->yManual});
                if (ctrl) {
                    double pivot  = d->screenToDataY(my);
                    double newRng = (d->vyMax - d->vyMin) * ZOOM_IN;
                    double loFrac = (pivot - d->vyMin) / (d->vyMax - d->vyMin);
                    d->vyMin   = pivot - loFrac * newRng;
                    d->vyMax   = d->vyMin + newRng;
                    d->yManual = true;
                } else {
                    double totalX = d->xViewMax - d->xViewMin;
                    double cur    = (d->xRangeWidth > 0.0) ? d->xRangeWidth : totalX;
                    d->xRangeWidth = std::max(cur * ZOOM_IN, totalX * 0.01);
                }
            } else if (e.wheel.y < 0) {
                // Zoom out — restore the previous zoom-in step
                if (!d->zoomHistory.empty()) {
                    auto &prev     = d->zoomHistory.back();
                    d->xRangeWidth = prev.xRangeWidth;
                    d->vyMin       = prev.vyMin;
                    d->vyMax       = prev.vyMax;
                    d->yManual     = prev.yManual;
                    d->zoomHistory.pop_back();
                }
            }
            break;
        }

        default: break;
        }
    }
    return d->open;
}

// ── Render ────────────────────────────────────────────────────────────────────
void PlotWidget::render() {
    d->updateView();

    SDL_SetRenderDrawColor(d->renderer, 10, 10, 14, 255);
    SDL_RenderClear(d->renderer);

    d->drawTitle();
    d->drawGrid();
    d->drawAxes();
    d->drawTraces();
    d->drawAxisLabels();
    d->drawLegend();

    SDL_RenderPresent(d->renderer);
}

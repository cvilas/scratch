#pragma once

#include "Trace.h"

#include <cstdint>
#include <memory>
#include <string>

/// SDL-based real-time plot window.
///
/// Typical usage:
/// \code
///   PlotWidget plot(900, 600, "My Plot");
///   plot.setAxisText(PlotWidget::AxisId::AxisX, "Time (s)");
///   auto& sig = plot.trace("Signal");
///
///   while (plot.processEvents()) {
///       sig.addData(t, value);
///       plot.render();
///   }
/// \endcode
///
/// Zoom controls (same as the SDL3 proof-of-concept):
///   Scroll          → zoom X axis
///   Ctrl + Scroll   → zoom Y axis
///   Double-click    → reset zoom
///   Drag legend     → move legend box
class PlotWidget {
public:
    enum class AxisId : uint8_t { AxisX, AxisY };

    explicit PlotWidget(int width = 900, int height = 600,
                        const std::string &title = "Plot");
    ~PlotWidget();

    PlotWidget(const PlotWidget &)            = delete;
    PlotWidget &operator=(const PlotWidget &) = delete;

    // ── Window title ─────────────────────────────────────────────────────
    void setTitleText(const std::string &title);
    [[nodiscard]] auto getTitleText() const -> std::string;

    // ── Axis labels ───────────────────────────────────────────────────────
    void setAxisText(AxisId id, const std::string &label);
    [[nodiscard]] auto getAxisText(AxisId id) const -> std::string;

    // ── Legend ────────────────────────────────────────────────────────────
    void enableLegend(bool on);
    [[nodiscard]] auto isLegendEnabled() const -> bool;

    // ── Traces ────────────────────────────────────────────────────────────
    /// Return the named trace; creates it (with the next palette colour)
    /// if one with that name does not already exist.
    [[nodiscard]] auto trace(const std::string &name) -> Trace &;

    // ── Event / render loop ───────────────────────────────────────────────
    /// Pump SDL events. Returns false when the window has been closed.
    bool processEvents();

    /// Render one frame into the window.
    void render();

    /// True while the window has not been closed.
    [[nodiscard]] bool isOpen() const;

private:
    struct Private;
    std::unique_ptr<Private> d;
};

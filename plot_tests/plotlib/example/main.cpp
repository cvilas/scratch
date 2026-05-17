#include <plotlib/PlotWidget.h>
#include <plotlib/Trace.h>

#include <SDL3/SDL.h>

#include <cmath>
#include <numbers>

int main() {
    PlotWidget plot(960, 600, "Real-time Plot – plotlib demo");
    plot.setAxisText(PlotWidget::AxisId::AxisX, "Time (s)");
    plot.setAxisText(PlotWidget::AxisId::AxisY, "Amplitude");
    plot.enableLegend(true);

    // Create three traces with different rendering styles
    auto &sine   = plot.trace("Sine");
    auto &cosine = plot.trace("Cosine");
    auto &beat   = plot.trace("Beat");

    cosine.setLineStyle(Trace::LineStyle::Step);
    beat.setPointStyle(Trace::PointStyle::Circle);
    beat.setLineStyle(Trace::LineStyle::Line);

    double t = 0.0;

    while (plot.processEvents()) {
        t += 0.033;   // ~30 Hz data rate

        using std::numbers::pi;
        sine  .addData(t,  std::sin(2.0 * pi * 0.5 * t));
        cosine.addData(t,  std::cos(2.0 * pi * 0.3 * t) * 0.7);
        beat  .addData(t, (std::sin(2.0 * pi * 1.0 * t)
                         + std::sin(2.0 * pi * 1.1 * t)) * 0.4);

        plot.render();
        // SDL_RenderPresent blocks on vsync; no extra delay needed.
    }

    return 0;
}

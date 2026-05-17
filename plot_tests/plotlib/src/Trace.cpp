#include "plotlib/Trace.h"

#include <algorithm>
#include <cassert>
#include <stdexcept>

// ── Private constructor ───────────────────────────────────────────────────────
Trace::Trace(std::string name)
    : m_name(std::move(name)), m_x(k_maxPoints), m_y(k_maxPoints) {}

// ── Accessors ─────────────────────────────────────────────────────────────────
auto Trace::getName() const -> const std::string & { return m_name; }

void Trace::setLineStyle(LineStyle s)  { m_lineStyle  = s; }
auto Trace::getLineStyle()  const -> LineStyle  { return m_lineStyle;  }

void Trace::setPointStyle(PointStyle s) { m_pointStyle = s; }
auto Trace::getPointStyle() const -> PointStyle { return m_pointStyle; }

auto Trace::xData() const -> const RingBuffer & { return m_x; }
auto Trace::yData() const -> const RingBuffer & { return m_y; }

// ── Data mutation ─────────────────────────────────────────────────────────────
void Trace::addData(double x, double y) {
    m_x.push_back(x);
    m_y.push_back(y);
    // RingBuffer overwrites the oldest element automatically when full
}

void Trace::setData(std::span<const double> x, std::span<const double> y) {
    assert(x.size() == y.size());
    m_x.assign(x.data(), x.size());
    m_y.assign(y.data(), y.size());
}

#pragma once

#include <cstdint>
#include <span>
#include <string>
#include <utility>
#include <vector>

/// Contiguous ring buffer (FIFO) with O(1) push and cache-friendly two-span iteration.
/// Pre-allocates capacity at construction so it never reallocates while running.
class RingBuffer {
public:
    explicit RingBuffer(size_t cap = 0) : m_data(cap), m_cap(cap) {}

    void push_back(double v) noexcept {
        if (m_size < m_cap) {
            m_data[m_size] = v;          // filling phase: head stays at 0
            ++m_size;
        } else {
            m_data[m_head] = v;          // full: overwrite oldest slot
            if (++m_head == m_cap) m_head = 0;
        }
    }

    [[nodiscard]] bool   empty()  const noexcept { return m_size == 0; }
    [[nodiscard]] size_t size()   const noexcept { return m_size; }

    double operator[](size_t i) const noexcept {
        size_t idx = m_head + i;
        if (idx >= m_cap) idx -= m_cap;
        return m_data[idx];
    }
    double front() const noexcept { return m_data[m_head]; }
    double back()  const noexcept {
        size_t i = m_head + m_size - 1;
        if (i >= m_cap) i -= m_cap;
        return m_data[i];
    }

    void clear() noexcept { m_head = 0; m_size = 0; }

    /// Replace all content with the last min(n, capacity) values from [ptr, ptr+n).
    void assign(const double *ptr, size_t n) noexcept {
        m_head = 0; m_size = 0;
        size_t skip = (n > m_cap) ? n - m_cap : 0;
        for (size_t i = skip; i < n; ++i) m_data[m_size++] = ptr[i];
    }

    /// Returns two contiguous spans in chronological order (oldest → newest).
    /// Iterating s1 then s2 visits every element once with no modulo arithmetic.
    [[nodiscard]]
    std::pair<std::span<const double>, std::span<const double>> spans() const noexcept {
        if (m_size == 0) return {{}, {}};
        const double *p    = m_data.data();
        size_t        tail = m_head + m_size;
        if (tail <= m_cap)
            return {{p + m_head, m_size}, {}};
        return {{p + m_head, m_cap - m_head}, {p, tail - m_cap}};
    }

private:
    std::vector<double> m_data;
    size_t m_head = 0;
    size_t m_size = 0;
    size_t m_cap  = 0;
};

class PlotWidget;

class Trace {
public:
    /// Style used to depict a data point on the graph
    enum class PointStyle : uint8_t {
        None,
        Dot,
        Cross,
        Plus,
        Circle,
        Disc,
        Square,
        Diamond,
        Star,
        Triangle,
        TriangleInverted,
        CrossSquare,
        PlusSquare,
        CrossCircle,
        PlusCircle,
        Peace,
    };

    /// Style used to join data points on a graph
    enum class LineStyle : uint8_t {
        None,      ///< Values not connected (only symbols)
        Lollipop,  ///< Vertical lines from baseline + optional symbol at tip
        Step,      ///< Staircase: step height is the value of the left data point
        Line,      ///< Values connected by straight line segments (default)
    };

    [[nodiscard]] auto getName() const -> const std::string &;

    void setLineStyle(LineStyle style);
    [[nodiscard]] auto getLineStyle() const -> LineStyle;

    void setPointStyle(PointStyle style);
    [[nodiscard]] auto getPointStyle() const -> PointStyle;

    /// Append a single (x, y) data point.
    void addData(double x, double y);

    /// Replace all data with the provided arrays (must be same length).
    void setData(std::span<const double> x, std::span<const double> y);

    // ── Internal access (PlotWidget only) ───────────────────────────────
    [[nodiscard]] auto xData() const -> const RingBuffer &;
    [[nodiscard]] auto yData() const -> const RingBuffer &;

private:
    friend class PlotWidget;
    explicit Trace(std::string name);

    std::string m_name;
    LineStyle   m_lineStyle  {LineStyle::Line};
    PointStyle  m_pointStyle {PointStyle::None};

    RingBuffer m_x;
    RingBuffer m_y;

    // Colour assigned by PlotWidget from its palette
    uint8_t m_r{0}, m_g{200}, m_b{80}, m_a{255};

    static constexpr size_t k_maxPoints = 100'000;
};

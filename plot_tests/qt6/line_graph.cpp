#include "line_graph.h"
#include <numbers>

static constexpr auto TIMEOUT = std::chrono::milliseconds(1);

LineGraph::LineGraph(QObject *parent) {
    Q_UNUSED(parent)
    line_series_ = new QLineSeries;
    line_series_->setWidth(2);
    line_series_->setColor(Qt::red);
    line_series_->setName("Sine");

    QObject::connect(&timer_, SIGNAL(timeout()), this, SLOT(onTimer()));
    timer_.start(TIMEOUT);
}

LineGraph::~LineGraph() {
    delete line_series_;
}

auto LineGraph::lineSeries() const -> QLineSeries * {
    return line_series_;
}

void LineGraph::clearSeries() {
    line_series_->clear();
}

void LineGraph::onClearSeries() {
    clearSeries();
}

void LineGraph::setLineSeries(QLineSeries *series) {
    if (line_series_ != series) {
        line_series_ = series;
        emit lineSeriesChanged();
    }
}

void LineGraph::onTimer() {
    static double t = 0;
    t += TIMEOUT.count();
    const auto pt = QPointF(t, 10*sin(2* std::numbers::pi * t /1000));
    line_series_->append(pt);
    emit lineSeriesChanged();
}

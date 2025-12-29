#pragma once

#include <QObject>
#include <QLineSeries>
#include <QTimer>

class LineGraph : public QObject {
    Q_OBJECT
    Q_PROPERTY(QLineSeries *lineSeries READ lineSeries WRITE setLineSeries NOTIFY lineSeriesChanged FINAL)
public:
    explicit LineGraph(QObject *parent = nullptr);
    ~LineGraph();

    auto lineSeries() const -> QLineSeries*;
    void setLineSeries(QLineSeries *series);
    void clearSeries();
Q_SIGNALS:
    void lineSeriesChanged();    
public Q_SLOTS:
    void onClearSeries();
private Q_SLOTS:
    void onTimer();
private:
    QLineSeries* line_series_;
    QTimer timer_;
};

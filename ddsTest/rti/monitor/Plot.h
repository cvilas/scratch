#ifndef PLOT_H
#define PLOT_H

#include <QWidget>
#include <QtCharts/QChartView>

class Plot : public QWidget
{
public:
    static constexpr int PLOT_LENGTH = 5000;
public:
    explicit Plot(QWidget *parent = 0);
    ~Plot();
    void init(const QString& topic);
    void display(double d);
private:
    QtCharts::QChartView _view;
    double _minVal;
    double _maxVal;
    bool _init;
};

#endif // PLOT_H

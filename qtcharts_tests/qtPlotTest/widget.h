#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QtCharts/QChartView>
#include <QTimer>
#include <QVector>
#include <QPointF>

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT
public:
    static constexpr double TIMEOUT_MS = 1;

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();
private slots:
    void onTimer();
private:
    Ui::Widget *ui;
    QtCharts::QChartView _view;
    QTimer _timer;
    QVector< QVector<QPointF> > _data;
};

#endif // WIDGET_H

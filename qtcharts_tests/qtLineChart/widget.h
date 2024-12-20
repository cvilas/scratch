#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QChartView>
#include <QTimer>
#include <QVector>
#include <QPointF>

class Widget : public QWidget {
    Q_OBJECT
public:
    explicit Widget(QWidget *parent = 0);
private slots:
    void onTimer();
private:
    QChartView _view;
    QTimer _timer;
    QVector< QVector<QPointF> > _data;
};

#endif // WIDGET_H

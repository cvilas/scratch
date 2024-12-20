#include "widget.h"
#include <QLayout>
#include <QLineSeries>
#include <QValueAxis>
#include <QList>
#include <QDebug>
#include <algorithm>
#include <cmath>

static constexpr auto TIMEOUT = std::chrono::milliseconds(1);

Widget::Widget(QWidget *parent) : QWidget(parent) {
    setLayout(new QVBoxLayout());
    layout()->addWidget(&_view);

    auto* pChart = _view.chart();
    auto* pAxisX = new QValueAxis;
    auto* pAxisY = new QValueAxis;
    for(int i = 0; i < 10; ++i)
    {
        auto* pSeries = new QLineSeries;
        pSeries->setUseOpenGL(true);
        pChart->addSeries(pSeries);
        pChart->setAxisX(pAxisX, pSeries);
        pChart->setAxisY(pAxisY, pSeries);
        _data.push_back(QVector<QPointF>());
    }
    QObject::connect(&_timer, SIGNAL(timeout()), this, SLOT(onTimer()));
    _timer.start(TIMEOUT);
}

void Widget::onTimer()
{
    static double t = 0;
    t += TIMEOUT.count();
    auto* pChart = _view.chart();
    auto* axisX = static_cast<QValueAxis*>(pChart->axisX());
    auto* axisY = static_cast<QValueAxis*>(pChart->axisY());

    QList<QAbstractSeries*> pSeriesList = pChart->series();
    for(int i = 0; i < pSeriesList.size(); ++i )
    {
        _data[i].append(QPointF(t, 10*i*sin(2*M_PI*i*t/1000)));
        static_cast<QLineSeries*>(pSeriesList[i])->replace(_data[i]);
    }

    axisX->setRange(std::max(0, static_cast<int>(_data[0].size())-10000), _data[0].size());
    axisY->setRange(-100,100);

    qDebug() << _data[0].size();
}

#include "widget.h"
#include "ui_widget.h"
#include <QLayout>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QList>
#include <QDebug>

#include <cmath>

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    setLayout(new QVBoxLayout());
    layout()->addWidget(&_view);

    QtCharts::QChart* pChart = _view.chart();
    QtCharts::QValueAxis* pAxisX = new QtCharts::QValueAxis;
    QtCharts::QValueAxis* pAxisY = new QtCharts::QValueAxis;
    for(int i = 0; i < 10; ++i)
    {
        QtCharts::QLineSeries* pSeries = new QtCharts::QLineSeries;
        pSeries->setUseOpenGL(true);
        pChart->addSeries(pSeries);
        pChart->setAxisX(pAxisX ,pSeries);
        pChart->setAxisY(pAxisY, pSeries);
        _data.push_back(QVector<QPointF>());
    }
    QObject::connect(&_timer, SIGNAL(timeout()), this, SLOT(onTimer()));
    _timer.start(TIMEOUT_MS);
}

Widget::~Widget()
{
    delete ui;
}


void Widget::onTimer()
{
    static double t = 0;
    t += TIMEOUT_MS;
    QtCharts::QChart* pChart = _view.chart();
    QtCharts::QValueAxis* axisX = static_cast<QtCharts::QValueAxis*>(pChart->axisX());
    QtCharts::QValueAxis* axisY = static_cast<QtCharts::QValueAxis*>(pChart->axisY());

    QList<QtCharts::QAbstractSeries*> pSeriesList = pChart->series();
    for(int i = 0; i < pSeriesList.size(); ++i )
    {
        _data[i].append(QPointF(t, 10*i*sin(2*M_PI*i*t/1000)));
        static_cast<QtCharts::QLineSeries*>(pSeriesList[i])->replace(_data[i]);
    }

    axisX->setRange(std::max(0,_data[0].size()-10000), _data[0].size());
    axisY->setRange(-100,100);

    qDebug() << _data[0].size();
}

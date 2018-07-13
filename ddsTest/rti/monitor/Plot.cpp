#include "Plot.h"
#include <QLayout>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QVector>
#include <sstream>

//---------------------------------------------------------------------------------------------------------------------
Plot::Plot(QWidget *parent)
//---------------------------------------------------------------------------------------------------------------------
    : QWidget(parent), _init(true)
{
    setLayout(new QVBoxLayout());
    layout()->addWidget(&_view);

    QtCharts::QChart* pChart = _view.chart();
    QtCharts::QValueAxis* pAxisX = new QtCharts::QValueAxis;
    QtCharts::QValueAxis* pAxisY = new QtCharts::QValueAxis;

    QtCharts::QLineSeries* pSeries = new QtCharts::QLineSeries;
    pSeries->setUseOpenGL(true);
    pChart->addSeries(pSeries);
    pChart->setAxisX(pAxisX ,pSeries);
    pChart->setAxisY(pAxisY, pSeries);
}

//---------------------------------------------------------------------------------------------------------------------
Plot::~Plot()
//---------------------------------------------------------------------------------------------------------------------
{

}

//---------------------------------------------------------------------------------------------------------------------
void Plot::init(const QString &name)
//---------------------------------------------------------------------------------------------------------------------
{
    QList<QtCharts::QAbstractSeries*> pSeriesList = _view.chart()->series();
    static_cast<QtCharts::QLineSeries*>(pSeriesList[0])->pointsVector().clear();
    _init = true;
    _view.chart()->series()[0]->setName(name);
}

//---------------------------------------------------------------------------------------------------------------------
void Plot::display(double d)
//---------------------------------------------------------------------------------------------------------------------
{
    QtCharts::QChart* pChart = _view.chart();
    QtCharts::QValueAxis* pAxisY = static_cast<QtCharts::QValueAxis*>(pChart->axisY());
    QtCharts::QValueAxis* pAxisX = static_cast<QtCharts::QValueAxis*>(pChart->axisX());

    QList<QtCharts::QAbstractSeries*> pSeriesList = pChart->series();
    QVector<QPointF> oldPoints = static_cast<QtCharts::QLineSeries*>(pSeriesList[0])->pointsVector();
    QVector<QPointF> points;

    int count = oldPoints.count();
    if (count < PLOT_LENGTH)
    {
        points = static_cast<QtCharts::QLineSeries*>(pSeriesList[0])->pointsVector();
    }
    else
    {
        _minVal = _maxVal;
        for (int i = count - PLOT_LENGTH; i < count; i++)
        {
            QPointF v = oldPoints.at(i);
            points.append(v);

            if(v.y() < _minVal) _minVal = v.y();
        }
        count = points.count();
    }

    int x = ((count == 0)?(0):(points.last().x()));
    points.append(QPointF(x+1, d));
    static_cast<QtCharts::QLineSeries*>(pSeriesList[0])->replace(points);

    pAxisX->setRange(points.at(0).x(), points.last().x());

    if(_init)
    {
        _minVal = d;
        _maxVal = d;
        _init = false;
    }
    else
    {
        if(d > _maxVal) _maxVal = d;
        if(d < _minVal) _minVal = d;
    }
    pAxisY->setRange(_minVal, _maxVal);

}

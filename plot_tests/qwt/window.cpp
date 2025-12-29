#include "window.h"

#include <cmath>  // for sine stuff
#include <QRandomGenerator>
#include <QStatusBar>
#include <qwt/qwt_scale_widget.h>
#include <qwt/qwt_plot_grid.h>
#include <qwt/qwt_plot.h>
#include <qwt/qwt_plot_curve.h>

Window::Window(QWidget* parent) : QMainWindow(parent)
{
  plot = new QwtPlot;
  setCentralWidget(plot);
  setup();
}

void Window::setup()
{
  QwtPlotGrid *grid = new QwtPlotGrid();
  grid->setPen( Qt::gray, 0.0, Qt::DotLine );
  grid->enableX( true );
  grid->enableXMin( true );
  grid->enableY( true );
  grid->enableYMin( false );
  grid->attach( plot );

  curve = new QwtPlotCurve;
  curve->setPen(QColor(40, 110, 255));
  curve->attach(plot);

  curve2 = new QwtPlotCurve;
  curve2->setPen(QColor(255, 110, 40));
  curve2->attach(plot);

  curve3 = new QwtPlotCurve;
  curve3->setPen(QColor(40, 255, 40));
  curve3->attach(plot);

  plot->setAxisAutoScale(QwtPlot::xBottom);
  plot->setAxisAutoScale(QwtPlot::yLeft);

  elapsedTime.start();
  
  // setup a timer that repeatedly calls MainWindow::realtimeDataSlot:
  connect(&dataTimer, SIGNAL(timeout()), this, SLOT(update()));
  dataTimer.start(0); // Interval 0 means to refresh as fast as possible
}

void Window::update()
{

  double key = elapsedTime.elapsed()/1000.0; // time elapsed since start of demo, in seconds
  static double lastPointKey = 0;

  if (key-lastPointKey > 0.002) // at most add point every 2 ms
  {
    xData.append(key);
    yData.append(qSin(key)+QRandomGenerator::global()->generateDouble()*1*qSin(key/0.3843));
    yData2.append(qCos(key)+QRandomGenerator::global()->generateDouble()*0.5*qSin(key/0.4364));
    yData3.append(qSin(key)+QRandomGenerator::global()->generateDouble()*0.5*qCos(key/0.4364));

    curve->setSamples(xData, yData);
    curve2->setSamples(xData, yData2);
    curve3->setSamples(xData, yData3);

    lastPointKey = key;

    plot->setAxisScale(QwtPlot::xBottom, key-8, key);
  }
  plot->replot();

  // calculate frames per second:
  static double lastFpsKey;
  static int frameCount;
  ++frameCount;
  if (key-lastFpsKey > 2) // average fps over 2 seconds
  {
    statusBar()->showMessage(
          QString("%1 FPS, Total Data points: %2")
          .arg(frameCount/(key-lastFpsKey), 0, 'f', 0)
          .arg(yData.size() + yData2.size() + yData3.size())
          , 0);
    lastFpsKey = key;
    frameCount = 0;
  }
}



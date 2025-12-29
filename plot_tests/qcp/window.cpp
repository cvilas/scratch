/***************************************************************************
**                                                                        **
**  QCustomPlot, an easy to use, modern plotting widget for Qt            **
**  Copyright (C) 2011-2018 Emanuel Eichhammer                            **
**                                                                        **
**  This program is free software: you can redistribute it and/or modify  **
**  it under the terms of the GNU General Public License as published by  **
**  the Free Software Foundation, either version 3 of the License, or     **
**  (at your option) any later version.                                   **
**                                                                        **
**  This program is distributed in the hope that it will be useful,       **
**  but WITHOUT ANY WARRANTY; without even the implied warranty of        **
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         **
**  GNU General Public License for more details.                          **
**                                                                        **
**  You should have received a copy of the GNU General Public License     **
**  along with this program.  If not, see http://www.gnu.org/licenses/.   **
**                                                                        **
****************************************************************************
**           Author: Emanuel Eichhammer                                   **
**  Website/Contact: http://www.qcustomplot.com/                          **
**             Date: 25.06.18                                             **
**          Version: 2.0.1                                                **
****************************************************************************/

/************************************************************************************************************
**                                                                                                         **
**  This is the example code for QCustomPlot.                                                              **
**                                                                                                         **
**  It demonstrates basic and some advanced capabilities of the widget. The interesting code is inside     **
**  the "setup(...)Demo" functions of MainWindow.                                                          **
**                                                                                                         **
**  In order to see a demo in action, call the respective "setup(...)Demo" function inside the             **
**  MainWindow constructor. Alternatively you may call setupDemo(i) where i is the index of the demo       **
**  you want (for those, see MainWindow constructor comments). All other functions here are merely a       **
**  way to easily create screenshots of all demos for the website. I.e. a timer is set to successively     **
**  setup all the demos and make a screenshot of the window area and save it in the ./screenshots          **
**  directory.                                                                                             **
**                                                                                                         **
*************************************************************************************************************/

#include "window.h"
#include "qcustomplot.h"
#include <QRandomGenerator>

Window::Window(QWidget *parent) : QMainWindow(parent)
{
  plot = new QCustomPlot;
#ifdef QCUSTOMPLOT_USE_OPENGL
  plot->setOpenGl(true);
#else
  plot->setOpenGl(false);
#endif
  setCentralWidget(plot);
  setup();

  qDebug() << "using opengl: " << plot->openGl();
}

void Window::setup()
{ 
  plot->addGraph(); // blue line
  plot->graph(0)->setPen(QPen(QColor(40, 110, 255)));
  plot->addGraph(); // red line
  plot->graph(1)->setPen(QPen(QColor(255, 110, 40)));
  plot->addGraph(); // green line
  plot->graph(2)->setPen(QPen(QColor(40, 255, 40)));

  QSharedPointer<QCPAxisTickerTime> timeTicker(new QCPAxisTickerTime);
  timeTicker->setTimeFormat("%h:%m:%s");
  plot->xAxis->setTicker(timeTicker);
  plot->axisRect()->setupFullAxesBox();
  
  // make left and bottom axes transfer their ranges to right and top axes:
  connect(plot->xAxis, SIGNAL(rangeChanged(QCPRange)), plot->xAxis2, SLOT(setRange(QCPRange)));
  connect(plot->yAxis, SIGNAL(rangeChanged(QCPRange)), plot->yAxis2, SLOT(setRange(QCPRange)));
  
  // setup a timer that repeatedly calls MainWindow::realtimeDataSlot:
  connect(&dataTimer, SIGNAL(timeout()), this, SLOT(update()));
  dataTimer.start(0); // Interval 0 means to refresh as fast as possible
}

void Window::update()
{
  static QElapsedTimer time = []{ QElapsedTimer t; t.start(); return t; }();

  double key = time.elapsed()/1000.0; // time elapsed since start of demo, in seconds
  static double lastPointKey = 0;
  if (key-lastPointKey > 0.002) // at most add point every 2 ms
  {
    // add data to lines:
    plot->graph(0)->addData(key, qSin(key)+QRandomGenerator::global()->generateDouble()*1*qSin(key/0.3843));
    plot->graph(1)->addData(key, qCos(key)+QRandomGenerator::global()->generateDouble()*0.5*qSin(key/0.4364));
    plot->graph(2)->addData(key, qSin(key)+QRandomGenerator::global()->generateDouble()*0.5*qCos(key/0.4364));

    lastPointKey = key;

    plot->xAxis->setRange(key, 8, Qt::AlignRight);
    plot->graph(0)->rescaleValueAxis();
    plot->graph(1)->rescaleValueAxis();
    plot->graph(2)->rescaleValueAxis();
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
          .arg(plot->graph(0)->data()->size()+plot->graph(1)->data()->size()+plot->graph(2)->data()->size())
          , 0);
    lastFpsKey = key;
    frameCount = 0;
  }
}




























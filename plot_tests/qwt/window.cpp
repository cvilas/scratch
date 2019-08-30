#include "window.h"
// #include "adcreader.h"

#include <cmath>  // for sine stuff
#include <QTime>
#include <QStatusBar>
#include <qwt/qwt_scale_widget.h>
#include <qwt/qwt_plot_grid.h>
#include <qwt/qwt_plot_glcanvas.h>
#include <qwt/qwt_plot_canvas.h>
#include <qwt/qwt_plot.h>
#include <qwt/qwt_plot_curve.h>

#define USE_OPENGL 0

#if USE_OPENGL
class GLCanvas: public QwtPlotGLCanvas
{
public:
    GLCanvas( QwtPlot *parent = NULL ): QwtPlotGLCanvas( parent )
    {
      setContentsMargins( 1, 1, 1, 1 );
    }

protected:
    virtual void paintEvent( QPaintEvent *event )
    {
        QPainter painter( this );
        //painter.setClipRegion( event->region() );

        QwtPlot *plot = qobject_cast< QwtPlot *>( parent() );
        if ( plot )
            plot->drawCanvas( &painter );

        painter.setPen( palette().foreground().color() );
        painter.drawRect( rect().adjusted( 1, 1, 0, 0 ) );
    }
};
#endif

Window::Window(QWidget* parent) : QMainWindow(parent)
{
  plot = new QwtPlot;

#if USE_OPENGL
  // use opengl canvas
  QwtPlotGLCanvas *plotCanvas = qobject_cast<QwtPlotGLCanvas *>( plot->canvas() );
  plotCanvas = new GLCanvas();
  plotCanvas->setPalette( QColor( "white" ) );
  plot->setCanvas( plotCanvas );
#endif

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
  curve->setPen(QColor(40, 110, 255), 1, Qt::PenStyle::DashLine);
  curve->attach(plot);

  curve2 = new QwtPlotCurve;
  curve2->setPen(QColor(255, 110, 40), 1, Qt::PenStyle::SolidLine);
  curve2->attach(plot);

  curve3 = new QwtPlotCurve;
  curve3->setPen(QColor(40, 255, 40), 1, Qt::PenStyle::DashDotLine);
  curve3->attach(plot);

  plot->setAxisAutoScale(QwtPlot::xBottom);
  plot->setAxisAutoScale(QwtPlot::yLeft);

  // setup a timer that repeatedly calls MainWindow::realtimeDataSlot:
  connect(&dataTimer, SIGNAL(timeout()), this, SLOT(update()));
  dataTimer.start(0); // Interval 0 means to refresh as fast as possible
}

void Window::update()
{
  static QTime time(QTime::currentTime());


  double key = time.elapsed()/1000.0; // time elapsed since start of demo, in seconds
  static double lastPointKey = 0;
  if (key-lastPointKey > 0.002) // at most add point every 2 ms
  {
    xData.append(key);
    yData.append(qSin(key)+qrand()/(double)RAND_MAX*1*qSin(key/0.3843));
    yData2.append(qCos(key)+qrand()/(double)RAND_MAX*0.5*qSin(key/0.4364));
    yData3.append(qSin(key)+qrand()/(double)RAND_MAX*0.5*qCos(key/0.4364));

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



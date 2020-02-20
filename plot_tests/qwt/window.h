#ifndef WINDOW_H
#define WINDOW_H

#include <QTimer>
#include <QElapsedTimer>
#include <QMainWindow>
#include <QVector>

class QwtPlot;
class QwtPlotCurve;

class Window : public QMainWindow
{
	Q_OBJECT

public:
  explicit Window(QWidget *parent = 0);
  ~Window() = default;
  void setup();

private slots:
  void update();

private:
  QTimer dataTimer;
  QElapsedTimer elapsedTime;

  QwtPlot      *plot;
	QwtPlotCurve *curve;
  QwtPlotCurve *curve2;
  QwtPlotCurve *curve3;

  QVector<double> xData{};
  QVector<double> yData{};
  QVector<double> yData2{};
  QVector<double> yData3{};
};

#endif // WINDOW_H

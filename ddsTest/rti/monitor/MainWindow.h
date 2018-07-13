#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "MonitorMessenger.h"
#include "Plot.h"
#include "DoubleBuffer.h"
#include <vector>
#include <QTimer>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    static constexpr int MESSENGER_DOMAIN_ID = 0;
    static constexpr int VIEW_UPDATE_PERIOD_MS = 33;

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    void logCallback(const AbstractLogVariable &var);
    void controlCallback(const AbstractControlVariable &var);

private slots:
    void onSelectLogVariable(const QString& topic);
    void onRefreshLogVariables();
    void onRefreshControlParameters();
    void onSelectControlVariable(const QString& topic);
    void onSliderValueChanged(int);
    void updateViews();

private:
    MonitorMessenger _messenger;
    std::string _monitoredLogName;
    std::string _monitoredControlName;
    DoubleBuffer<double> _buffer;
    ControlVariableT<double> _control;
    QTimer _updateTimer;
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H

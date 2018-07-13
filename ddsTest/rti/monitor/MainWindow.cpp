#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QDebug>
#include "LogVariable.h"
#include "ControlVariable.h"

//---------------------------------------------------------------------------------------------------------------------
MainWindow::MainWindow(QWidget *parent)
//---------------------------------------------------------------------------------------------------------------------
    : QMainWindow(parent),
      _messenger(MESSENGER_DOMAIN_ID),
      ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->pCtrlSetter->setMinimum(0);
    ui->pCtrlSetter->setMaximum(1000);
    ui->pCtrlSetter->setSingleStep(1);

    setWindowTitle("DDS Test");
    connect(ui->pLogRefresh, SIGNAL(clicked(bool)), this, SLOT(onRefreshLogVariables()));
    connect(ui->pLogList, SIGNAL(activated(QString)), this, SLOT(onSelectLogVariable(QString)));
    connect(&_updateTimer, SIGNAL(timeout()), this, SLOT(updateViews()));

    connect(ui->pCtrlRefresh, SIGNAL(clicked(bool)), this, SLOT(onRefreshControlParameters()));
    connect(ui->pCtrlList, SIGNAL(activated(QString)), this, SLOT(onSelectControlVariable(QString)));
    connect(ui->pCtrlSetter, SIGNAL(valueChanged(int)), this, SLOT(onSliderValueChanged(int)));

    _updateTimer.start(VIEW_UPDATE_PERIOD_MS);

}

//---------------------------------------------------------------------------------------------------------------------
MainWindow::~MainWindow()
//---------------------------------------------------------------------------------------------------------------------
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::onRefreshLogVariables()
//---------------------------------------------------------------------------------------------------------------------
{
    const std::vector<std::string> topics = _messenger.logVariableNames();
    ui->pLogList->clear();
    ui->pLogList->addItem("-");
    for(const auto& item : topics)
    {
        ui->pLogList->addItem(item.c_str());
    }
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::onRefreshControlParameters()
//---------------------------------------------------------------------------------------------------------------------
{
    const std::vector<std::string> topics = _messenger.controlVariableNames();
    ui->pCtrlList->clear();
    ui->pCtrlList->addItem("-");
    for(const auto& item : topics)
    {
        ui->pCtrlList->addItem(item.c_str());
    }
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::logCallback(const AbstractLogVariable &var)
//---------------------------------------------------------------------------------------------------------------------
{
    ///@todo: this crashes if the ui update functions are called. The potential issue is that the callback
    /// is run in the context of the messenger (separate thread) and that messes up the gui update.
    /// Solution: copy var into a local threadsafe buffer and raise a signal to update UI elements.

    const LogVariableT<double>& log = dynamic_cast< const LogVariableT<double>& >(var);
    _buffer.push(log.value());
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::controlCallback(const AbstractControlVariable &var)
//---------------------------------------------------------------------------------------------------------------------
{
    const ControlVariableT<double>& ctrl = dynamic_cast<const ControlVariableT<double>& >(var);
    _control.fromBytes(var.toBytes());
    ui->pCtrlMin->setText(QString::number(ctrl.min()));
    ui->pCtrlMax->setText(QString::number(ctrl.max()));
    ui->pCtrlSetValue->setText(QString::number(ctrl.value()));

    if( ui->pCtrlSetter->signalsBlocked() )
    {
        ui->pCtrlSetter->setValue( ((ctrl.value() - ctrl.min()) * 1000.)/(ctrl.max() - ctrl.min()) );
        ui->pCtrlDemand->setText(QString::number(ctrl.value()));
        ui->pCtrlSetter->blockSignals(false);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::onSliderValueChanged(int value)
//---------------------------------------------------------------------------------------------------------------------
{
    double demand = (value/1000.) * (_control.max() - _control.min()) + _control.min();
    ui->pCtrlDemand->setText(QString::number(demand));
    _messenger.control<double>(_monitoredControlName, demand);
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::updateViews()
//---------------------------------------------------------------------------------------------------------------------
{
    const std::vector<double>& logs = _buffer.get();
    for(const double& d : logs)
    {
        ui->pLogWatch->setText(QString::number(d));
        ui->pLogPlot->display(d);
    }
    _buffer.swap();
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::onSelectLogVariable(const QString& topic)
//---------------------------------------------------------------------------------------------------------------------
{
    _messenger.unmonitorLogVariable(_monitoredLogName);
    _buffer.clear();
    ui->pLogWatch->clear();
    ui->pLogPlot->init(topic);

    if(topic != "-") // first element in the list, and is invalid
    {
        _monitoredLogName = topic.toStdString();
        _messenger.monitorLogVariable<double>(_monitoredLogName, std::bind(&MainWindow::logCallback, this, std::placeholders::_1) );
    }
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::onSelectControlVariable(const QString& topic)
//---------------------------------------------------------------------------------------------------------------------
{
    _messenger.unmonitorControlVariable(_monitoredControlName);
    if(topic != "-") // first element in the list, and is invalid
    {
        ui->pCtrlSetter->blockSignals(true);
        _monitoredControlName = topic.toStdString();
        _messenger.monitorControlVariable<double>(_monitoredControlName, std::bind(&MainWindow::controlCallback, this, std::placeholders::_1) );
    }
}

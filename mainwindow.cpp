#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->checkDC, &QCheckBox::stateChanged,
            this, [this](int){this->uiParamChanged();});
}

void MainWindow::setBl0940(Bl0940Handler* handler)
{
    m_handler = handler;

    connect(m_handler->getBleSerial(), &BleSerial::connected,
            this, &MainWindow::deviceConnected);
    connect(m_handler->getBleSerial(), &BleSerial::connectionFailed,
            this, &MainWindow::deviceConnectionFailed);
    connect(m_handler->getBleSerial(), &BleSerial::disconnected,
            this, &MainWindow::deviceDisconnected);

    connect(m_handler, &Bl0940Handler::readElecData,
            this, &MainWindow::receivedElecData, Qt::BlockingQueuedConnection);
}

void MainWindow::deviceConnected()
{
    ui->labelConnection->setText(m_handler->getBleSerial()->deviceName());
}

void MainWindow::deviceConnectionFailed()
{
    ui->labelConnection->setText("ERROR...");
}

void MainWindow::deviceDisconnected()
{
    ui->labelConnection->setText("---");
}

void MainWindow::receivedElecData(BL0940 blData)
{
    qDebug("MainWindow::receivedElecData(): being called. divider: %f, r_shunt: %f",
           blData.voltage_divider, blData.r_shunt_ohm);
    ui->labelVoltage->setText(QString::number(blData.voltage, 'f', 1) + " V");
    ui->labelCurrent->setText(QString::number(blData.current, 'f', 3) + " A");
    ui->labelPhaseAngle->setText(QString::number(blData.phase_angle, 'f', 0) + "°");
    ui->labelPowerFactor->setText(QString::number(blData.power_factor, 'f', 3));
    ui->labelPower->setText(QString::number(blData.power, 'f', 3) + " W");
    ui->labelEnergy->setText(QString::number(blData.energy, 'f', 4) + " kWh");
    ui->labelTempInternal->setText(QString::number(blData.temp_internal, 'f', 0) + " ℃");
}

void MainWindow::uiParamChanged()
{
    BL0940 param = m_handler->getParam();
    param.setting_filter = (ui->checkDC->checkState()? BL0940_Filter_DC_Pass
                                                     : BL0940_Filter_AC_Pass);
    m_handler->setParam(param);
}

MainWindow::~MainWindow()
{
    delete ui;
}

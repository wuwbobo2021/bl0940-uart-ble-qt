#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

void MainWindow::receivedElecData(BL0940 blData)
{
    qDebug("MainWindow::receivedElecData(): being called. divider: %f, r_shunt: %f",
           blData.voltage_divider, blData.r_shunt_ohm);
    ui->labelVoltage->setText(QString::number(blData.voltage, 'f', 1));
    ui->labelCurrent->setText(QString::number(blData.current, 'f', 3));
    ui->labelPhaseAngle->setText(QString::number(blData.phase_angle, 'f', 0));
    ui->labelPowerFactor->setText(QString::number(blData.power_factor, 'f', 3));
    ui->labelPower->setText(QString::number(blData.power, 'f', 3));
    ui->labelEnergy->setText(QString::number(blData.energy, 'f', 4));
    ui->labelTempInternal->setText(QString::number(blData.temp_internal, 'f', 0));
}

MainWindow::~MainWindow()
{
    delete ui;
}

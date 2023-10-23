#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "bl0940handler.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    void setBl0940(Bl0940Handler* handler);
    ~MainWindow();

private slots:
    void deviceConnected();
    void deviceConnectionFailed();
    void deviceDisconnected();
    void receivedElecData(BL0940 blData);

    void uiParamChanged();

private:
    Ui::MainWindow *ui;
    Bl0940Handler* m_handler;
};
#endif // MAINWINDOW_H

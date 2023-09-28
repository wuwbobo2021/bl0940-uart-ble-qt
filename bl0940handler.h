#ifndef BL0940HANDLER_H
#define BL0940HANDLER_H

#include "bl0940.h"
#include "devicehandler.h"
#include <QThread>

class Bl0940Handler : public QThread
{
    Q_OBJECT
signals:
    void bleWrite(QByteArray data);
    void readElecData(BL0940 blData);

public:
    Bl0940Handler(DeviceHandler* ble);
    void run() override;

private:
    volatile bool m_stop = false;
};

Bl0940Handler* createBl0940Handler(DeviceHandler* ble);

#endif // BL0940HANDLER_H

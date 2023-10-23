// for use with bl0940 (bl0940.c, bl0940.h)

#ifndef BL0940HANDLER_H
#define BL0940HANDLER_H

#include "bl0940.h"
#include "bleserial.h"
#include <QThread>

class Bl0940Handler : public QThread
{
    Q_OBJECT
signals:
    void readElecData(BL0940 blData);

public:
    Bl0940Handler(BleSerial* ble);
    void run() override;

    BleSerial* getBleSerial() const;
    BL0940 getParam() const;
    void setParam(BL0940 param);

private:
    volatile bool m_flagSetParam = true;
    /*volatile*/ BL0940 m_param;

    volatile bool m_flagStop = false;
};

Bl0940Handler* createBl0940Handler(BleSerial* ble);

#endif // BL0940HANDLER_H

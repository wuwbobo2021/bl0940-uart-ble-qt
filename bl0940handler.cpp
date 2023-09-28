#include "bl0940handler.h"
//#include <QtDebug>

static BL0940 bl0940;
static DeviceHandler* bleHandler = nullptr;

static Bl0940Handler* threadBl0940 = nullptr;

Bl0940Handler* createBl0940Handler(DeviceHandler* ble)
{
    if (! threadBl0940)
        threadBl0940 = new Bl0940Handler(ble);
    threadBl0940->setPriority(QThread::LowPriority);
    return threadBl0940;
}

void Bl0940Handler::run()
{
    qDebug("Bl0940Handler::run(): started.");

    QThread::msleep(500);
    bool suc = bl0940_apply_settings(&bl0940);
    if (! suc)
        qDebug("Bl0940Handler::run(): bl0940_apply_settings() failed: error %d.", bl0940.error);

    m_stop = false;
    while (! m_stop) {
        QThread::msleep(500);
        if (! bl0940_get_readings(&bl0940)) {
            qDebug("Bl0940Handler::run(): failed to read elec data.");
            continue;
        }
        qDebug("Bl0940Handler::run(): got elec data.");
        emit readElecData(bl0940);
    }
}

Bl0940Handler::Bl0940Handler(DeviceHandler* ble)
{
    bl0940.r_shunt_ohm = 0.001;
    bl0940.voltage_divider = 3795;
    bl0940.calc_stable_angle = true;
    bl0940.setting_av_time = BL0940_Av_Time_800ms;

    bleHandler = ble;
    connect(this, &Bl0940Handler::bleWrite, ble, &DeviceHandler::bleWrite);
    connect(ble, &DeviceHandler::bleConnected, this, [this](){this->start();});
    connect(ble, &DeviceHandler::bleDisconnected, this, [this](){m_stop = true; this->wait();});
}

bool bl0940_uart_send(uint8_t port_num, const void* data, uint8_t cnt)
{
    if (! bleHandler) return false;
    qDebug("bl0940_uart_send(): being called.");
    bleHandler->receivedData = QByteArray("");
    emit threadBl0940->bleWrite(QByteArray((const char*)data, cnt));
    return true;
}

uint8_t bl0940_uart_receive(uint8_t port_num, void* data, uint8_t cnt, uint16_t timeout_ms)
{
    if (!bleHandler || !data || !cnt || !timeout_ms) return 0;
    qDebug("bl0940_uart_receive(): being called.");

    int ms = 0;
    while (bleHandler->receivedData.length() < cnt) {
        if (ms > timeout_ms) break;
        QThread::msleep(1); ms++;
    }

    int rec_cnt = bleHandler->receivedData.length();
    if (rec_cnt > cnt) rec_cnt = cnt;
    memcpy(data, bleHandler->receivedData.constData(), rec_cnt);
    bleHandler->receivedData.remove(0, rec_cnt);

    if (rec_cnt) qDebug("bl0940_uart_receive(): %d bytes received.", rec_cnt);
    return rec_cnt;
}

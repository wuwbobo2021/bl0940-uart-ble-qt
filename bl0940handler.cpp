#include "bl0940handler.h"

static BL0940 bl0940;
static BleSerial* bleSerial = nullptr;
static Bl0940Handler* threadBl0940 = nullptr;

Bl0940Handler* createBl0940Handler(BleSerial* ble)
{
    if (! threadBl0940)
        threadBl0940 = new Bl0940Handler(ble);
    threadBl0940->setPriority(QThread::LowPriority);
    return threadBl0940;
}

Bl0940Handler::Bl0940Handler(BleSerial* ble)
{
    m_param = {
        .r_shunt_ohm = 0.001,
        .voltage_divider = 3795,
        .calc_stable_angle = true,
        .setting_filter = BL0940_Filter_AC_Pass,
        .setting_av_time = BL0940_Av_Time_400ms
    };
    m_flagSetParam = true;

    bleSerial = ble;
    bleSerial->setBaudrate(4800);
    connect(bleSerial, &BleSerial::connected,
            this, [this](){this->start();});
    connect(bleSerial, &BleSerial::disconnected,
            this, [this](){m_flagStop = true; this->wait();});
}

BleSerial* Bl0940Handler::getBleSerial() const
{
    return bleSerial;
}

BL0940 Bl0940Handler::getParam() const
{
    return m_param;
}

void Bl0940Handler::setParam(BL0940 param)
{
    m_param = param; QThread::msleep(1);
    m_flagSetParam = true;
}

void Bl0940Handler::run()
{
    qDebug("Bl0940Handler::run(): started.");
    QThread::msleep(500);

    m_flagStop = false;
    while (! m_flagStop) {
        QThread::msleep(200);

        if (m_flagSetParam) {
            bl0940 = m_param;
            bool suc = bl0940_apply_settings(&bl0940);
            if (! suc) {
                qDebug("Bl0940Handler::run(): set params failed: error %d.", bl0940.error);
                continue;
            }
            m_flagSetParam = false;
        }

        if (! bl0940_get_readings(&bl0940)) {
            qDebug("Bl0940Handler::run(): failed to read elec data.");
            continue;
        }

        emit readElecData(bl0940);
    }
}

// extern (for bl0940), port_num is unused for single BleSerial

bool bl0940_uart_send(uint8_t port_num, const void* data, uint8_t cnt)
{
    if (! bleSerial) return false;
    qDebug("bl0940_uart_send(): being called.");
    bleSerial->readAll(); //clear read buffer
    bleSerial->write((const char*)data, cnt);
    return true;
}

uint8_t bl0940_uart_receive(uint8_t port_num, void* data, uint8_t cnt, uint16_t timeout_ms)
{
    if (!bleSerial || !data || !cnt || !timeout_ms) return 0;
    qDebug("bl0940_uart_receive(): being called.");

    int ms = 0;
    while (bleSerial->bytesAvailable() < cnt) {
        if (ms > timeout_ms) break;
        QThread::msleep(1); ms += 1;
    }

    int rec_cnt = bleSerial->bytesAvailable();
    if (! rec_cnt) return 0;
    if (rec_cnt > cnt) rec_cnt = cnt;
    bleSerial->read((char*)data, rec_cnt);
    qDebug("bl0940_uart_receive(): %d bytes received.", rec_cnt);
    return rec_cnt;
}

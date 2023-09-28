#ifndef DEVICEHANDLER_H
#define DEVICEHANDLER_H

#include <QObject>
#include <QBluetoothDeviceDiscoveryAgent>
#include <QLowEnergyController>
#include <QLowEnergyService>
#include <QBluetoothDeviceInfo>

class DeviceHandler : public QObject
{
    Q_OBJECT
public:
    QBluetoothAddress m_deviceAddress = QBluetoothAddress("00:E0:02:12:00:54");
    QBluetoothUuid m_serviceUuid = QBluetoothUuid((quint16)0xA00A),
                   m_readUuid    = QBluetoothUuid((quint16)0xB003),
                   m_writeUuid   = QBluetoothUuid((quint16)0xB002);

    QByteArray receivedData;

    explicit DeviceHandler(QObject* parent = nullptr);

signals:
    void bleDeviceNotFound();
    void bleConnected();
    void bleDisconnected();

public slots:
    void startScan();
    bool bleWrite(QByteArray data);

private slots:
    void addDevice(const QBluetoothDeviceInfo &);
    void scanError(QBluetoothDeviceDiscoveryAgent::Error error);
    void scanFinished();
    void serviceDiscovered(const QBluetoothUuid &);
    void controllerStateChanged(QLowEnergyController::ControllerState state);
    void controllerDisconnected();
    void serviceStateChanged(QLowEnergyService::ServiceState s);
    void characteristicChanged(const QLowEnergyCharacteristic& c, const QByteArray& value);

private:
    QBluetoothDeviceDiscoveryAgent* m_bleDiscover = nullptr;
    QLowEnergyController* m_deviceControl = nullptr;
    QBluetoothDeviceInfo* m_deviceInfo = nullptr;
    QLowEnergyService* m_deviceService = nullptr;
    volatile bool m_connected = false, m_discovering = false;

    void deleteDevice();
};

#endif // DEVICEHANDLER_H

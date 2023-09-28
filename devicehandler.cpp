#include "devicehandler.h"
//#include <QtDebug>

DeviceHandler::DeviceHandler(QObject* parent)
    : QObject{parent}
{
    m_bleDiscover = new QBluetoothDeviceDiscoveryAgent(this);
    m_bleDiscover->setLowEnergyDiscoveryTimeout(15000);

    connect(m_bleDiscover, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered, this, &DeviceHandler::addDevice);
    connect(m_bleDiscover,
            static_cast<void (QBluetoothDeviceDiscoveryAgent::*)(QBluetoothDeviceDiscoveryAgent::Error)>
                (&QBluetoothDeviceDiscoveryAgent::error),
            this, &DeviceHandler::scanError);

    connect(m_bleDiscover, &QBluetoothDeviceDiscoveryAgent::finished, this, &DeviceHandler::scanFinished);
    connect(m_bleDiscover, &QBluetoothDeviceDiscoveryAgent::canceled, this, &DeviceHandler::scanFinished);
}

void DeviceHandler::startScan()
{
    this->deleteDevice();
    m_bleDiscover->start(QBluetoothDeviceDiscoveryAgent::LowEnergyMethod);
    this->m_discovering = true;
}

void DeviceHandler::addDevice(const QBluetoothDeviceInfo& info)
{
    if ((info.coreConfigurations() & QBluetoothDeviceInfo::LowEnergyCoreConfiguration) == 0)
        return;

    if (info.address() != m_deviceAddress) {
        qDebug("DeviceHandler::addDevice(): wrong address.");
        return;
    }
    m_deviceInfo = new QBluetoothDeviceInfo(info);
    m_bleDiscover->stop(); this->m_discovering = false;
    while (m_bleDiscover->isActive());
    m_deviceControl = QLowEnergyController::createCentral(*m_deviceInfo, this);
    m_deviceControl->setRemoteAddressType(QLowEnergyController::PublicAddress);
    connect(m_deviceControl, &QLowEnergyController::connected,
            this, [this](){m_deviceControl->discoverServices();});
    connect(m_deviceControl, &QLowEnergyController::serviceDiscovered,
            this, &DeviceHandler::serviceDiscovered);
    connect(m_deviceControl, &QLowEnergyController::stateChanged,
            this, &DeviceHandler::controllerStateChanged);
    connect(m_deviceControl, &QLowEnergyController::disconnected,
            this, &DeviceHandler::controllerDisconnected);

    m_deviceControl->connectToDevice();
}

void DeviceHandler::scanError(QBluetoothDeviceDiscoveryAgent::Error error)
{
    qDebug("DeviceHandler::scanError()");
    this->m_discovering = false;
    emit this->bleDeviceNotFound();
}

void DeviceHandler::scanFinished()
{
    qDebug("DeviceHandler::scanFinished()");
    this->m_discovering = false;
    if (! m_deviceInfo) emit this->bleDeviceNotFound();
}

void DeviceHandler::serviceDiscovered(const QBluetoothUuid& uuid)
{
    if (uuid != m_serviceUuid) {
        qDebug("DeviceHandler::serviceDiscovered(): wrong service uuid.");
        return;
    }
    qDebug("DeviceHandler::serviceDiscovered(): correct service uuid.");
    m_deviceService = m_deviceControl->createServiceObject(m_serviceUuid, this);
    connect(m_deviceService, &QLowEnergyService::stateChanged,
            this, &DeviceHandler::serviceStateChanged);
    connect(m_deviceService, &QLowEnergyService::characteristicChanged,
            this, &DeviceHandler::characteristicChanged);
    m_deviceService->discoverDetails();
    qDebug("DeviceHandler::serviceDiscovered(): discoverDetails() called.");
}

void DeviceHandler::controllerStateChanged(QLowEnergyController::ControllerState state)
{
    if (state == QLowEnergyController::ClosingState
    ||  state == QLowEnergyController::UnconnectedState) {
        if (m_deviceService)
            qDebug("DeviceHandler::controllerStateChanged(): disconnected.");
        this->deleteDevice();
    }
}

void DeviceHandler::controllerDisconnected()
{
    if (! m_deviceService) return;
    qDebug("DeviceHandler::controllerDisconnected(): disconnected.");
    this->deleteDevice();
}

void DeviceHandler::serviceStateChanged(QLowEnergyService::ServiceState s)
{
    if (s == QLowEnergyService::ServiceDiscovered) {
        const QLowEnergyCharacteristic ch = m_deviceService->characteristic(m_readUuid);
        if (! ch.isValid()) {
            qDebug("DeviceHandler::serviceStateChanged(): failed to get read characteristic.");
            DeviceHandler::deleteDevice(); return;
        }
        QLowEnergyDescriptor desc = ch.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration);
        if (! desc.isValid()) {
            qDebug("DeviceHandler::serviceStateChanged(): failed to configure read characteristic.");
            DeviceHandler::deleteDevice(); return;
        }
        m_deviceService->writeDescriptor(desc, QByteArray::fromHex("0100"));
        m_connected = true; emit this->bleConnected();
        qDebug("DeviceHandler::serviceStateChanged(): characteristic found, connected.");
    }
    else if (s == QLowEnergyService::InvalidService) {
        qDebug("DeviceHandler::serviceStateChanged(): disconnected.");
        this->deleteDevice();
    }
}

void DeviceHandler::characteristicChanged(const QLowEnergyCharacteristic& c, const QByteArray& value)
{
    if (c.uuid() != m_readUuid) return;
    this->receivedData += value;
    qDebug("DeviceHandler::characteristicChanged(): value readed.");
}

bool DeviceHandler::bleWrite(QByteArray data)
{
    if (!m_deviceService || !data.length()) return false;
    const QLowEnergyCharacteristic ch = m_deviceService->characteristic(m_writeUuid);
    if (! ch.isValid()) return false;
    m_deviceService->writeCharacteristic(ch, data, QLowEnergyService::WriteWithoutResponse);
    qDebug("DeviceHandler::bleWrite(): success.");
    return true;
}

void DeviceHandler::deleteDevice()
{
    qDebug("DeviceHandler::deleteDevice(): being called.");
    if (m_connected) {
        m_connected = false; emit this->bleDisconnected();
    } else if (m_deviceService)
        emit this->bleDeviceNotFound();

    if (m_deviceService) {
        m_deviceService->deleteLater(); m_deviceService = nullptr;
    }
    if (m_deviceControl) {
        m_deviceControl->disconnectFromDevice();
        m_deviceControl->deleteLater(); m_deviceControl = nullptr;
    }

    delete m_deviceInfo; m_deviceInfo = nullptr;

    qDebug("DeviceHandler::deleteDevice(): completed.");
}

#ifndef EFM32LOADER_H
#define EFM32LOADER_H

#ifndef EFM32LOADER_SERIAL
#define EFM32LOADER_SERIAL  1
#endif

#ifndef EFM32LOADER_BLE
#define EFM32LOADER_BLE     1
#endif

#include <QObject>

#if EFM32LOADER_SERIAL
#include <QSerialPort>
class XMODEM;
#endif

#if EFM32LOADER_BLE
#include <QBluetoothDeviceDiscoveryAgent>
#include <QBluetoothDeviceInfo>
#include <QLowEnergyController>
#include <QLowEnergyService>
#include <QLowEnergyCharacteristic>
#include <QLowEnergyDescriptor>
#include <QLowEnergyHandle>
#endif

class GeckoLoader : public QObject
{
    Q_OBJECT
public:
    enum Transport
    {
        TransportUART,
        TransportUSB,
        TransportBLE
    };

    explicit GeckoLoader(QObject *parent = 0);
    void setTransport(Transport transport);
    void resetData();

#if EFM32LOADER_SERIAL
    QSerialPort *serialPort() { return _serialPort; }
    void setBootEnablePolarity(bool high);
#endif

#if EFM32LOADER_BLE
    QLowEnergyController *bleController() { return _bleController; }
    void setBleController(QLowEnergyController *controller);
    void setBleServiceAndCharacteristic(QLowEnergyService *service, QBluetoothUuid charUuid);
    void setBleUUIDs(QBluetoothUuid serviceUUID, QBluetoothUuid charUUID);
#endif

signals:
    void output(QString);

public slots:
#if EFM32LOADER_SERIAL
    bool open(const QString &portName);
    void close();
    bool upload(const QString &filePath);
#endif

#if EFM32LOADER_BLE
    void connectBle(const QString &addr);
    void disconnectBle();
    bool detectBle();
    bool uploadBle(const QString &filePath);
#endif

private slots:
#if EFM32LOADER_BLE
    void _slotBleConnected();
    void _slotBleError(QLowEnergyController::Error err);
    void _slotBleDisconnected();
    void _slotBleServiceDiscovered(QBluetoothUuid uuid);
    void _slotBleDiscoveryFinished();

    void _slotBleCharWritten(QLowEnergyCharacteristic c, QByteArray data);
    void _slotBleCharChanged(QLowEnergyCharacteristic c, QByteArray data);
#endif

private:
    enum DataType
    {
        dataTypeString,
        dataTypeByteArray
    };

    Transport _transport;

#if EFM32LOADER_SERIAL
    QSerialPort *_serialPort;
    XMODEM      *_xmodem;
    bool        _bootEnablePolarity;

    bool waitForChipID();
    bool waitForReady();
    bool waitForData(int timeout);

    void enterBoot();
    void exitBoot();
    bool detect();
    void reset();
#endif

#if EFM32LOADER_BLE
    QLowEnergyController        *_bleController;
    QLowEnergyService           *_bleService;
    QBluetoothUuid              _bleServiceUUID;
    QBluetoothUuid              _bleCharUUID;

    QList<QLowEnergyService*>   _bleServices;
    bool _bleCharWritten;
    bool _bleCharChanged;
    bool _bleBootDetected;
    bool _bleBootReady;
    bool _bleUploadError;

    QLowEnergyService* _findService(QBluetoothUuid uuid);
    void _writeChar(QString data, DataType dataType);
    void _writeChar(QByteArray data);
    bool _waitCharWritten(int timeout_ms);
    bool _waitCharChanged(int timeout_ms);
    bool _bleBegin();
    bool _bleSend(char *buf, int count, bool flush);
    void _bleEnd();
#endif



};

#endif // EFM32LOADER_H

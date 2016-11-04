#include "geckoloader.h"
#include "xmodem.h"



#include <QDebug>
#include <QEventLoop>
#include <QTimer>
#include <QFile>
#include <QElapsedTimer>

GeckoLoader::GeckoLoader(QObject *parent) :
    QObject(parent)
{
#if EFM32LOADER_BLE
    _bleController = 0;
    _transport = TransportBLE;
#endif

#if EFM32LOADER_SERIAL
    _serialPort = new QSerialPort(this);
    _xmodem = new XMODEM(_serialPort, this);
    _bootEnablePolarity = true;
    _transport = TransportUART;
    connect(_xmodem, SIGNAL(output(QString)), this, SIGNAL(output(QString)));
#endif
}

void GeckoLoader::setTransport(Transport transport)
{
    _transport = transport;
}

void GeckoLoader::resetData()
{
#if EFM32LOADER_BLE
    _bleCharChanged     = false;
    _bleCharWritten     = false;
    _bleBootDetected    = false;
    _bleBootReady       = false;
#endif
}

#if EFM32LOADER_SERIAL

void GeckoLoader::setBootEnablePolarity(bool high)
{
    _bootEnablePolarity = high;
}

bool GeckoLoader::open(const QString &portName)
{
    QSerialPort *sp = _serialPort;

    if(!sp->isOpen())
    {
        sp->setPortName(portName);
        if(sp->open(QSerialPort::ReadWrite))
        {
            sp->setBaudRate(115200);
            sp->setDataBits(QSerialPort::Data8);
            sp->setParity(QSerialPort::NoParity);
            sp->setStopBits(QSerialPort::OneStop);
            sp->setFlowControl(QSerialPort::NoFlowControl);

            emit output(tr("Connected"));
        }
        else
        {
            QString errMsg = tr("Failed to open port. ") + sp->errorString();
            emit output(errMsg);
            return false;
        }
    }
    else
    {
        emit output(tr("Port is already opened"));
        return false;
    }

    return true;
}

void GeckoLoader::close()
{
    emit output(tr("Disconnected"));
    _serialPort->close();
}

bool GeckoLoader::detect()
{
    char byteBuf;
    bool detected;

    if(_transport == TransportUART)
    {
        byteBuf = 'U';
        int tries = 10;
        QEventLoop eventLoop;
        QTimer timer;
        timer.setInterval(10);
        timer.setSingleShot(false);
        connect(&timer, SIGNAL(timeout()), &eventLoop, SLOT(quit()));
        timer.start();
        while(tries--)
        {
            _serialPort->write(&byteBuf, 1);
            eventLoop.exec();
        }
        timer.stop();
        detected = waitForChipID();
    }
    else if(_transport == TransportUSB)
    {
        byteBuf = 'i';
        _serialPort->write(&byteBuf, 1);
        detected = waitForChipID();
    }
    else if(_transport == TransportBLE)
    {

    }
    else
    {
        qDebug() << "ERROR | Unknown transport type";
    }

    return detected;
}

bool GeckoLoader::upload(const QString &filePath)
{
    QFile file(filePath);
    if(!file.open(QFile::ReadOnly))
    {
        emit output(tr("Failed to open .bin file:") + filePath + file.errorString());
        return false;
    }

    bool success;
    char byteBuf;

    QElapsedTimer elapsedTimer;

    enterBoot();

    elapsedTimer.start();

    if(!detect())
    {
        exitBoot();
        return false;
    }

    // Enter upload mode (XMODEM)
    byteBuf = 'u';
    _serialPort->write(&byteBuf, 1);
    waitForReady();

    emit output("Uploading...");

    // Send file through XMODEM-CRC protocol
    success = _xmodem->sendFile(filePath);
    emit output(QString().sprintf("Elapsed time: %.3f seconds", (double)elapsedTimer.elapsed()/1000.0));
    _serialPort->readAll();
    exitBoot();
    emit output(tr("Done"));
    return success;
}

void GeckoLoader::enterBoot()
{
    // RTS - RSTN
    // DTR - DBG

    QEventLoop eventLoop;
    QTimer timer;
    timer.setSingleShot(true);
    connect(&timer, SIGNAL(timeout()), &eventLoop, SLOT(quit()));

    if(_bootEnablePolarity == true)
        _serialPort->setDataTerminalReady(false);
    else
        _serialPort->setDataTerminalReady(true);
    timer.start(10);
    eventLoop.exec();

    reset();
}

void GeckoLoader::exitBoot()
{
    QEventLoop eventLoop;
    QTimer timer;
    timer.setSingleShot(true);
    connect(&timer, SIGNAL(timeout()), &eventLoop, SLOT(quit()));

    if(_bootEnablePolarity == true)
        _serialPort->setDataTerminalReady(true);
    else
        _serialPort->setDataTerminalReady(false);
    timer.start(10);
    eventLoop.exec();
    reset();
}

void GeckoLoader::reset()
{
    QEventLoop eventLoop;
    QTimer timer;
    timer.setSingleShot(true);
    connect(&timer, SIGNAL(timeout()), &eventLoop, SLOT(quit()));

    _serialPort->setRequestToSend(true);
    timer.start(100);
    eventLoop.exec();
    _serialPort->setRequestToSend(false);
    timer.start(100);
    eventLoop.exec();
}

bool GeckoLoader::waitForChipID()
{
    emit output("Waiting for chip ID");
    while(1)
    {
        if(!waitForData(2000))
        {
            emit output("Unable to receive chip ID");
            return false;
        }
        else
        {
            QString line = QString(_serialPort->readLine());
            if(line.contains("ChipID"))
            {
                emit output(tr("Bootloader detected"));
                return true;
            }
        }
    }
}

bool GeckoLoader::waitForReady()
{
    while(1)
    {
        if(!waitForData(3500))
        {
            emit output("Unable to receive 'C'");
            return false;
        }
        else
        {
            QString line = QString(_serialPort->readLine());
            if(line.contains("C"))
            {
                emit output(tr("Ready"));
                return true;
            }
        }
    }
}

bool GeckoLoader::waitForData(int timeout)
{
    QEventLoop eventLoop;
    QTimer timer;

    timer.setInterval(timeout);
    timer.setSingleShot(true);

    connect(_serialPort, SIGNAL(readyRead()), &eventLoop, SLOT(quit()));
    connect(&timer, SIGNAL(timeout()), &eventLoop, SLOT(quit()));

    if(_serialPort->bytesAvailable() > 0)
        return true;
    else
    {
        timer.start();
        eventLoop.exec();
        return (_serialPort->bytesAvailable() > 0);
    }
}
#endif //EFM32LOADER_SERIAL

#if EFM32LOADER_BLE

void GeckoLoader::setBleController(QLowEnergyController *controller)
{
    _bleController = controller;
    connect(_bleController, SIGNAL(error(QLowEnergyController::Error)),
            this, SLOT(_slotBleError(QLowEnergyController::Error)));
}

void GeckoLoader::setBleServiceAndCharacteristic(QLowEnergyService *service, QBluetoothUuid charUuid)
{
    _bleService = service;
    _bleServiceUUID = service->serviceUuid();
    _bleCharUUID = charUuid;

    connect(_bleService, SIGNAL(characteristicWritten(QLowEnergyCharacteristic,QByteArray)),
            this, SLOT(_slotBleCharWritten(QLowEnergyCharacteristic,QByteArray)));
    connect(_bleService, SIGNAL(characteristicChanged(QLowEnergyCharacteristic,QByteArray)),
            this, SLOT(_slotBleCharChanged(QLowEnergyCharacteristic,QByteArray)));
}

void GeckoLoader::setBleUUIDs(QBluetoothUuid serviceUUID, QBluetoothUuid charUUID)
{
    _bleServiceUUID = serviceUUID;
    _bleCharUUID = charUUID;
}

void GeckoLoader::connectBle(const QString &addr)
{
    emit output("Connecting to BLE device " + addr);
    if(_bleController != 0)
        delete _bleController;

    if(_bleServices.count() > 0)
    {
        qDeleteAll(_bleServices.begin(), _bleServices.end());
    }

    _bleController = new QLowEnergyController(QBluetoothAddress(addr));

    connect(_bleController, SIGNAL(connected()),
            this, SLOT(_slotBleConnected()));
    connect(_bleController, SIGNAL(error(QLowEnergyController::Error)),
            this, SLOT(_slotBleError(QLowEnergyController::Error)));
    connect(_bleController, SIGNAL(disconnected()),
            this, SLOT(_slotBleDisconnected()));
    connect(_bleController, SIGNAL(serviceDiscovered(QBluetoothUuid)),
            this, SLOT(_slotBleServiceDiscovered(QBluetoothUuid)));
    connect(_bleController, SIGNAL(discoveryFinished()),
            this, SLOT(_slotBleDiscoveryFinished()));

    _bleController->setRemoteAddressType(QLowEnergyController::RandomAddress);
    _bleController->connectToDevice();
}

void GeckoLoader::disconnectBle()
{
    if(_bleController == 0)
        return;

    emit output("Disconnecting from BLE device");
    _bleController->disconnectFromDevice();
}

bool GeckoLoader::detectBle()
{
    _bleBootDetected = false;
    _writeChar("d", dataTypeString);
    _waitCharChanged(3000);
    if(_bleBootDetected)
    {
        emit output("EFM32 bootloader found");
    }
    else
    {
        emit output("Failed to find EFM32 bootloader");
    }
    return _bleBootDetected;
}

bool GeckoLoader::uploadBle(const QString &filePath)
{
    QFile file(filePath);

    if(!file.open(QFile::ReadOnly))
    {
        emit output("Failed to open file " + filePath);
        return false;
    }

    if(!_bleBegin())
    {
        emit output("EFM32 bootloader failed to begin");
        return false;
    }

    emit output("Uploading...");

    _bleUploadError = false;

    QEventLoop eventLoop;
    QElapsedTimer elapsedTimer;
    elapsedTimer.start();

    char dataBuffer[16];
    int totalBytesRead = 0;

    while(!file.atEnd() && !_bleUploadError)
    {
        int bytesRead = file.read(dataBuffer, 16);
        totalBytesRead += bytesRead;

        emit output(QString().sprintf("[%6lu / %6lu]", totalBytesRead, file.size()));

        _bleSend(dataBuffer, bytesRead, file.atEnd());

        eventLoop.processEvents();
    }

    _bleEnd();

    emit output("Done");
    emit output(QString().sprintf("Elapsed time: %.3f seconds", (double)elapsedTimer.elapsed()/1000.0));
    emit output(QString("Total bytes sent: %1").arg(totalBytesRead));

    return false;
}

void GeckoLoader::_slotBleConnected()
{
    emit output("Connected to BLE device");
    emit output("Discovering services...");
    _bleController->discoverServices();
}

void GeckoLoader::_slotBleError(QLowEnergyController::Error err)
{
    emit output("BLE error: " + _bleController->errorString());
}

void GeckoLoader::_slotBleDisconnected()
{
    emit output("Disconnected from BLE device");
    if(_bleServices.count() > 0)
    {
        qDeleteAll(_bleServices.begin(), _bleServices.end());
    }
}

void GeckoLoader::_slotBleServiceDiscovered(QBluetoothUuid uuid)
{
    emit output("Service discovered: " + uuid.toString());

    QEventLoop eventLoop;
    QLowEnergyService *service = _bleController->createServiceObject(uuid);
    connect(service, SIGNAL(stateChanged(QLowEnergyService::ServiceState)),
            &eventLoop, SLOT(quit()));

    _bleServices.append(service);
    service->discoverDetails();

    while(service->state() != QLowEnergyService::ServiceDiscovered)
    {
        eventLoop.exec();
    }

    foreach(QLowEnergyCharacteristic bleChar, service->characteristics())
    {
        emit output(" Char: " + bleChar.uuid().toString());
    }

    delete service;
}

void GeckoLoader::_slotBleDiscoveryFinished()
{
    qDebug() << __FUNCTION__;
    return;
}

void GeckoLoader::_slotBleCharWritten(QLowEnergyCharacteristic c, QByteArray data)
{
    _bleCharWritten = true;
}

void GeckoLoader::_slotBleCharChanged(QLowEnergyCharacteristic c, QByteArray data)
{
    _bleCharChanged = true;
    qDebug() << "char changed" << c.uuid().toString() << data.toHex().toUpper();

    QString dataStr(data);
    if(dataStr == "kd")         _bleBootDetected    = true;
    else if(dataStr == "kb")    _bleBootReady       = true;
    else if(dataStr.at(0) == 'e')
    {
        _bleUploadError = true;
        emit output(QString("BLE boot error (%1)").arg(QString(data.toHex()).toUpper()));
    }
}

QLowEnergyService *GeckoLoader::_findService(QBluetoothUuid uuid)
{
    foreach(QLowEnergyService *s, _bleServices)
    {
        if(s->serviceUuid() == uuid)
            return s;
    }
    return 0;
}

void GeckoLoader::_writeChar(QString data, GeckoLoader::DataType dataType)
{
    QString dataToSendStr = data;
    QByteArray dataToSend;

    switch(dataType)
    {
    case dataTypeString: // String
        dataToSend = dataToSendStr.toUtf8();
        break;
    case dataTypeByteArray: // Byte array
        dataToSend = QByteArray::fromHex(dataToSendStr.toUtf8());
        break;
    default:
        qDebug() << "ERROR | unknown data type";
    }

    QLowEnergyService *s = _bleService;
    if(s == 0)
    {
        qDebug() << "ERROR | Failed to get service";
        return;
    }

    QLowEnergyCharacteristic c = s->characteristic(_bleCharUUID);
    if(!c.isValid())
    {
        qDebug() << "ERROR | Failed to get characteristic";
        return;
    }

    _bleCharWritten = false;
    _bleCharChanged = false;
    s->writeCharacteristic(c, dataToSend);
}

void GeckoLoader::_writeChar(QByteArray data)
{
    QLowEnergyService *s = _bleService;
    if(s == 0)
    {
        qDebug() << "ERROR | Failed to get service";
        return;
    }

    QLowEnergyCharacteristic c = s->characteristic(_bleCharUUID);
    if(!c.isValid())
    {
        qDebug() << "ERROR | Failed to get characteristic";
        return;
    }

    _bleCharWritten = false;
    _bleCharChanged = false;
    s->writeCharacteristic(c, data);
}

bool GeckoLoader::_waitCharWritten(int timeout_ms)
{
    QLowEnergyService *s = _bleService;
    if(s == 0)
    {
        qDebug() << "ERROR | Failed to get service";
        return false;
    }

    QEventLoop eventLoop;
    QTimer timer;

    connect(s, SIGNAL(characteristicWritten(QLowEnergyCharacteristic,QByteArray)),
            &eventLoop, SLOT(quit()));
    connect(&timer, SIGNAL(timeout()), &eventLoop, SLOT(quit()));

    timer.setSingleShot(true);
    timer.start(timeout_ms);

    while(timer.isActive() && !_bleCharWritten)
    {
        eventLoop.exec();
    }

    if(!timer.isActive())
    {
        emit output("WARNING: Failed to get charWritten in " + QString::number(timeout_ms) + "ms");
    }

    return timer.isActive();
}

bool GeckoLoader::_waitCharChanged(int timeout_ms)
{
    QLowEnergyService *s = _bleService;
    if(s == 0)
    {
        qDebug() << "ERROR | Failed to get service";
        return false;
    }

    QEventLoop eventLoop;
    QTimer timer;

    connect(s, SIGNAL(characteristicChanged(QLowEnergyCharacteristic,QByteArray)),
            &eventLoop, SLOT(quit()));
    connect(&timer, SIGNAL(timeout()), &eventLoop, SLOT(quit()));

    timer.setSingleShot(true);
    timer.start(timeout_ms);

    while(timer.isActive() && !_bleCharChanged)
    {
        eventLoop.exec();
    }

    if(!timer.isActive())
    {
        emit output("WARNING: Failed to get charChanged in " + QString::number(timeout_ms) + "ms");
    }

    return timer.isActive();
}

bool GeckoLoader::_bleBegin()
{
    _bleBootReady = false;
    _writeChar("b", dataTypeString);
    _waitCharChanged(3000);
    return _bleBootReady;
}

bool GeckoLoader::_bleSend(char *buf, int count, bool flush)
{
    QByteArray dataToSend;
    char send_cmd = flush ? 'f' : 's';
    dataToSend.append(send_cmd);
    dataToSend.append(buf, count);

//    qDebug() << QString("BLE SEND (%1bytes): %2")
//                .arg(count)
//                .arg(QString(QByteArray(buf, count).toHex()));

    _writeChar(dataToSend);
    return _waitCharWritten(2000);
}

void GeckoLoader::_bleEnd()
{
    _writeChar("e", dataTypeString);
    _waitCharWritten(3000);
}
#endif // EFM32LOADER_BLE

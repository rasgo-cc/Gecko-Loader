#include "geckoloader.h"
#include "xmodem.h"

#include <QDebug>
#include <QSerialPort>
#include <QEventLoop>
#include <QTimer>
#include <QFile>
#include <QElapsedTimer>

GeckoLoader::GeckoLoader(QObject *parent) :
    QObject(parent)
{
    _serialPort = new QSerialPort(this);
    _xmodem = new XMODEM(_serialPort, this);
    _bootEnablePolarity = true;
    _transport = TransportUART;
    connect(_xmodem, SIGNAL(output(QString)), this, SIGNAL(output(QString)));
}

void GeckoLoader::setBootEnablePolarity(bool high)
{
    _bootEnablePolarity = high;
}

void GeckoLoader::setTransport(Transport transport)
{
    _transport = transport;
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
        int tries = 3;
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
    else
    {
        byteBuf = 'i';
        _serialPort->write(&byteBuf, 1);
        detected = waitForChipID();
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



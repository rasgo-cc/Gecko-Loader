/*
 * QkThings LICENSE
 * The open source framework and modular platform for smart devices.
 * Copyright (C) 2014 <http://qkthings.com>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "efm32loader.h"
#include "xmodem.h"

#include <QDebug>
#include <QSerialPort>
#include <QEventLoop>
#include <QTimer>
#include <QFile>
#include <QElapsedTimer>

EFM32Loader::EFM32Loader(QObject *parent) :
    QObject(parent)
{
    m_serialPort = new QSerialPort(this);
    m_xmodem = new XMODEM(m_serialPort, this);
    _bootEnablePolarity = true;
    connect(m_xmodem, SIGNAL(output(QString)), this, SIGNAL(output(QString)));
}

void EFM32Loader::setBootEnablePolarity(bool high)
{
    _bootEnablePolarity = high;
}

bool EFM32Loader::open(const QString &portName)
{
    QSerialPort *sp = m_serialPort;

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

void EFM32Loader::close()
{
    emit output(tr("Disconnected"));
    m_serialPort->close();
}

bool EFM32Loader::detect()
{
    char byteBuf;

    m_serialPort->readAll();
    enterBoot();

    byteBuf = 'U';
    m_serialPort->write(&byteBuf, 1);
    bool detected = waitForChipID();

    m_serialPort->readAll();
    exitBoot();

    return detected;
}

bool EFM32Loader::upload(const QString &filePath)
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

    // Autobaud sync
    byteBuf = 'U';
    int tries = 3;
    QEventLoop eventLoop;
    QTimer timer;
    timer.setInterval(5);
    timer.setSingleShot(false);
    connect(&timer, SIGNAL(timeout()), &eventLoop, SLOT(quit()));
    timer.start();
    while(tries--)
    {
        m_serialPort->write(&byteBuf, 1);
        eventLoop.exec();
    }

    if(!waitForChipID())
    {
        exitBoot();
        return false;
    }

    // Enter upload mode (XMODEM)
    byteBuf = 'u';
    m_serialPort->write(&byteBuf, 1);
    waitForReady();

    emit output("Uploading...");

    // Send file through XMODEM-CRC protocol
    success = m_xmodem->sendFile(filePath);
    emit output(QString().sprintf("Elapsed time: %.3f seconds", (double)elapsedTimer.elapsed()/1000.0));
    m_serialPort->readAll();
    exitBoot();
    emit output(tr("Done"));
    return success;
}

void EFM32Loader::enterBoot()
{
    // RTS - RSTN
    // DTR - DBG

    QEventLoop eventLoop;
    QTimer timer;
    timer.setSingleShot(true);
    connect(&timer, SIGNAL(timeout()), &eventLoop, SLOT(quit()));

    if(_bootEnablePolarity == true)
        m_serialPort->setDataTerminalReady(false);
    else
        m_serialPort->setDataTerminalReady(true);
    timer.start(10);
    eventLoop.exec();

    reset();
}

void EFM32Loader::exitBoot()
{
    QEventLoop eventLoop;
    QTimer timer;
    timer.setSingleShot(true);
    connect(&timer, SIGNAL(timeout()), &eventLoop, SLOT(quit()));

    if(_bootEnablePolarity == true)
        m_serialPort->setDataTerminalReady(true);
    else
        m_serialPort->setDataTerminalReady(false);
    timer.start(10);
    eventLoop.exec();
    reset();
}

void EFM32Loader::reset()
{
    QEventLoop eventLoop;
    QTimer timer;
    timer.setSingleShot(true);
    connect(&timer, SIGNAL(timeout()), &eventLoop, SLOT(quit()));

    m_serialPort->setRequestToSend(true);
    timer.start(100);
    eventLoop.exec();
    m_serialPort->setRequestToSend(false);
    timer.start(100);
    eventLoop.exec();
}

bool EFM32Loader::waitForChipID()
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
            QString line = QString(m_serialPort->readLine());
            if(line.contains("ChipID"))
            {
                emit output(tr("Bootloader detected"));
                return true;
            }
        }
    }
}

bool EFM32Loader::waitForReady()
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
            QString line = QString(m_serialPort->readLine());
            if(line.contains("C"))
            {
                emit output(tr("Ready"));
                return true;
            }
        }
    }
}

bool EFM32Loader::waitForData(int timeout)
{
    QEventLoop eventLoop;
    QTimer timer;

    timer.setInterval(timeout);
    timer.setSingleShot(true);

    connect(m_serialPort, SIGNAL(readyRead()), &eventLoop, SLOT(quit()));
    connect(&timer, SIGNAL(timeout()), &eventLoop, SLOT(quit()));

    if(m_serialPort->bytesAvailable() > 0)
        return true;
    else
    {
        timer.start();
        eventLoop.exec();
        return (m_serialPort->bytesAvailable() > 0);
    }
}



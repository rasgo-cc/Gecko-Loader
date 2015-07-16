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

#ifndef EFM32LOADER_H
#define EFM32LOADER_H

#include <QObject>

class QSerialPort;
class XMODEM;

class EFM32Loader : public QObject
{
    Q_OBJECT
public:
    explicit EFM32Loader(QObject *parent = 0);

    QSerialPort *serialPort() { return m_serialPort; }
    void setBootEnablePolarity(bool high);

signals:
    void output(QString);

public slots:
    bool open(const QString &portName);
    void close();
    bool detect();
    bool upload(const QString &filePath);

private:
    QSerialPort *m_serialPort;
    XMODEM *m_xmodem;
    bool _bootEnablePolarity;

    bool waitForChipID();
    bool waitForReady();
    bool waitForData(int timeout);

    void enterBoot();
    void exitBoot();
    void reset();
};

#endif // EFM32LOADER_H

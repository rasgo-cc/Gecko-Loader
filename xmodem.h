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

#ifndef XMODEM_H
#define XMODEM_H

#include <QObject>

class QSerialPort;

class XMODEM : public QObject
{
    Q_OBJECT
public:
    explicit XMODEM(QSerialPort *sp, QObject *parent = 0);

signals:
    void output(QString message);

public slots:
    bool sendFile(const QString &filePath);


private:
    enum
    {
        SOH = 0x01,
        EOT = 0x04,
        ACK = 0x06,
        NAK = 0x15,
        ETB = 0x17,
        CAN = 0x18,
        C = 0x43
    };

    QSerialPort *m_serialPort;

    void sendPacket(char *data, int count);
    int calculateCRC(char *ptr, int count);
    int waitACK(int timeout);

    int m_packetNumber;

};

#endif // XMODEM_H

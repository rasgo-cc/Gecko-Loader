/*
 * XMODEM-CRC implementation according to:
 * http://web.mit.edu/6.115/www/miscfiles/amulet/amulet-help/xmodem.htm
 */

#include "xmodem.h"

#include <QDebug>

#include <QSerialPort>
#include <QFile>
#include <QEventLoop>
#include <QTimer>

XMODEM::XMODEM(QSerialPort *sp, QObject *parent) :
    QObject(parent)
{
    m_serialPort = sp;
}
void XMODEM::_readyRead()
{
    qDebug() << "readyRead:" << m_serialPort->readAll().toHex();
}

bool XMODEM::sendFile(const QString &filePath)
{
    QFile file(filePath);

    if(!file.open(QFile::ReadOnly))
    {
        qDebug() << "failed to open file" << filePath;
        return false;
    }

    char dataBuffer[128];
    m_packetNumber = 1;

    int totalBytesRead = 0;

    m_serialPort->readAll();

    bool uploadError = false;

    _lastPacketSent = QByteArray();

    while(!file.atEnd() && !uploadError)
    {
        int bytesRead = file.read(dataBuffer, 128);
        totalBytesRead += bytesRead;

        for(int i = bytesRead; i < 128; i++)
            dataBuffer[i] = 0xFF; // pad buffer with 0xFF

        //qDebug() << "packet:" << m_packetNumber << "bytes transfered:" << QString().sprintf("%10lu / %10lu", totalBytesRead, file.size());
        emit output(QString().sprintf("[%6lu / %6lu]", totalBytesRead, file.size()));

        sendPacket(dataBuffer, 128);
        uploadError = !waitACK(1000);

//        QEventLoop eventLoop;
//        QTimer timer;
//        timer.setSingleShot(true);
//        timer.start(1500);
//        while(timer.isActive())
//        {
//            QByteArray dummyPacket;
//            uint8_t pkt_num = (m_packetNumber - 1);
//            dummyPacket.append(XMODEM::SOH);
//            dummyPacket.append(pkt_num);
//            dummyPacket.append(0xFF - pkt_num);
//            dummyPacket.append(128, '.');
//            dummyPacket.append(2, '0');
//            m_serialPort->write(dummyPacket);
//            waitACK(1000);
//        }
        //connect(&timer, SIGNAL(timeout()), &eventLoop, SLOT(quit()));
        //eventLoop.exec();

//        if(m_packetNumber == 3)
//        {
//        connect(m_serialPort, SIGNAL(readyRead()), this, SLOT(_readyRead()));
//        while(1)
//            eventLoop.exec();
//        }
    }

    if(!uploadError)
    {
        dataBuffer[0] = (char) XMODEM::EOT;
        m_serialPort->write(dataBuffer, 1);
        waitACK(1000);
    }

    qDebug() << "file sent";
    qDebug() << m_serialPort->readAll();

    file.close();

    return true;
}

void XMODEM::sendPacket(char *data, int count)
{
    QSerialPort *sp = m_serialPort;
    QByteArray packet;
    int crc = calculateCRC(data, count);

    packet.append(XMODEM::SOH);
    packet.append(m_packetNumber);
    packet.append(0xFF - m_packetNumber);
    packet.append(data, count);

    packet.append((char) ((crc >> 8) & 0xFF));
    packet.append((char) (crc & 0xFF));

    _lastPacketSent = packet;

    qDebug() << m_packetNumber << QString(QByteArray(data,count).toHex().left(8)).toUpper();

    m_packetNumber++;
    if(m_packetNumber > 255)
        m_packetNumber = 0;

    sp->write(packet);
}

bool XMODEM::waitACK(int timeout)
{
    //qDebug() << "waitACK";
    int retries = 4;
    QByteArray rxData;

    QTimer timer;
    QEventLoop eventLoop;

    timer.setInterval(3500);

    connect(&timer, SIGNAL(timeout()), &eventLoop, SLOT(quit()));
    connect(m_serialPort, SIGNAL(readyRead()), &eventLoop, SLOT(quit()));

    while(retries-- > 0)
    {
        eventLoop.exec();
        rxData = m_serialPort->readAll();
        if(rxData.size() > 0)
        {
            //qDebug() << " rx:" << rxData.toHex();
            break;
        }
    }

    if(retries == 0 && rxData.size() == 0)
    {
        qDebug() << "failed to get ack";
        emit output(tr("XMODEM: Failed to get ACK"));
        return -1;
    }

    int code = rxData.at(0);
    switch(code)
    {
//    case XMODEM::ACK: qDebug() << "ACK"; break;
//    case XMODEM::NAK: qDebug() << "NAK"; break;
    case XMODEM::ACK:
    case XMODEM::NAK:
        break;
    default:
        qDebug() << "unknown ACK code:" << QString().sprintf("%02X",code);
        emit output(tr("XMODEM: Unknown ACK code ") + QString().sprintf("%02X",code));
    }
    return code == XMODEM::ACK;
}

int XMODEM::calculateCRC(char *ptr, int count)
{
    int  crc;
    char i;
    crc = 0;
    while (--count >= 0)
    {
        crc = crc ^ (int) *ptr++ << 8;
        i = 8;
        do
        {
            if (crc & 0x8000)
                crc = crc << 1 ^ 0x1021;
            else
                crc = crc << 1;
        } while(--i);
    }
    return (crc);
}

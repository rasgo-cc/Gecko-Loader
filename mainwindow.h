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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#ifdef EFM32_LOADER_GUI
#include <QMainWindow>
#include <QBluetoothDeviceDiscoveryAgent>

namespace Ui {
class MainWindow;
}

class GeckoLoader;
class QSerialPort;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void slotHelp();
    void slotReloadSerialPorts();
    void slotBrowse();
    void slotUpload();
    void slotConnect();
    void slotTransport();
    void slotSendASCII();
    void slotDataReady();

    void slotScanBLE();
    void slotConnectBLE();
    void slotTestBLE();
    void slotDeviceDiscovered(QBluetoothDeviceInfo info);
    void slotDeviceScanError(QBluetoothDeviceDiscoveryAgent::Error err);
    void slotDeviceScanFinished();

    void log(const QString &text);
    void updateInterface();

private:
    Ui::MainWindow *ui;

    GeckoLoader *loader;
    QSerialPort *serialPort;
    QBluetoothDeviceDiscoveryAgent *bleDiscoveryAgent;
    bool _connected;

    void readSettings();
};

#endif // EFM32_LOADER_GUI

#endif // MAINWINDOW_H

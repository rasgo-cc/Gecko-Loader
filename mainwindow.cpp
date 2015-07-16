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

#ifdef EFM32_LOADER_GUI

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "efm32loader.h"
#include "helpdialog.h"
#include <QSerialPortInfo>
#include <QFileDialog>
#include <QSettings>
#include <QSerialPort>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->lineASCII->hide();
    ui->textLog->setFont(QFont("Courier New", 9));

    loader = new EFM32Loader(this);
    serialPort = loader->serialPort();
    m_connected = false;

    readSettings();
    slotReloadSerialPorts();

    connect(loader, SIGNAL(output(QString)), this, SLOT(log(QString)));
    connect(serialPort, SIGNAL(readyRead()), this, SLOT(slotDataReady()));
    connect(ui->buttonHelp, SIGNAL(clicked()), this, SLOT(slotHelp()));
    connect(ui->buttonReload, SIGNAL(clicked()), this, SLOT(slotReloadSerialPorts()));
    connect(ui->buttonBrowse, SIGNAL(clicked()), this, SLOT(slotBrowse()));
    connect(ui->buttonDetect, SIGNAL(clicked()), this, SLOT(slotDetect()));
    connect(ui->buttonUpload, SIGNAL(clicked()), this, SLOT(slotUpload()));
    connect(ui->buttonConnect, SIGNAL(clicked()), this, SLOT(slotConnect()));

    connect(ui->lineASCII, SIGNAL(returnPressed()), this, SLOT(slotSendASCII()));

    setWindowTitle("EFM32 Loader");
    updateInterface();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::readSettings()
{
    QSettings settings;

    ui->lineFile->setText(settings.value("lastBinaryFile").toString());
}

void MainWindow::log(const QString &text)
{
    ui->textLog->appendPlainText(text);
}

void MainWindow::slotHelp()
{
    HelpDialog dialog;
    dialog.exec();
}

void MainWindow::slotReloadSerialPorts()
{
    QStringList list;
    foreach(QSerialPortInfo info, QSerialPortInfo::availablePorts())
    {
        QString portName = info.portName();
        list.append(portName);
    }
    ui->comboPort->clear();
    ui->comboPort->addItems(list);
    updateInterface();
}

void MainWindow::slotBrowse()
{
    QString filePath = QFileDialog::getOpenFileName(this, tr("Select a binary file"), QString(), "*.bin");
    ui->lineFile->setText(filePath);

    QSettings settings;
    settings.setValue("lastBinaryFile", QVariant(filePath));
}

void MainWindow::slotConnect()
{
    if(!m_connected)
    {
        m_connected = loader->open(ui->comboPort->currentText());
    }
    else
    {
        loader->close();
        m_connected = false;
    }
    updateInterface();
}

void MainWindow::slotDetect()
{
    disconnect(serialPort, SIGNAL(readyRead()), this, SLOT(slotDataReady()));
    loader->detect();
    connect(serialPort, SIGNAL(readyRead()), this, SLOT(slotDataReady()));
}

void MainWindow::slotUpload()
{
    disconnect(serialPort, SIGNAL(readyRead()), this, SLOT(slotDataReady()));
    if(ui->comboBootEnPol->currentText() == "HIGH")
        loader->setBootEnablePolarity(true);
    else
        loader->setBootEnablePolarity(false);
    loader->upload(ui->lineFile->text());
    connect(serialPort, SIGNAL(readyRead()), this, SLOT(slotDataReady()));
}

void MainWindow::slotSendASCII()
{
    QString text = ui->lineASCII->text();
    serialPort->write(text.toLatin1());
}

void MainWindow::slotDataReady()
{
    while(serialPort->bytesAvailable() > 0)
        ui->textLog->appendPlainText(serialPort->readAll());
}

void MainWindow::updateInterface()
{
    ui->buttonConnect->setDisabled(ui->comboPort->count() == 0);
    ui->buttonDetect->setEnabled(m_connected);
    ui->buttonUpload->setEnabled(m_connected);

    if(m_connected)
        ui->buttonConnect->setText(tr("Disconnect"));
    else
        ui->buttonConnect->setText(tr("Connect"));

    ui->comboPort->setEnabled(!m_connected);
}

#endif //EFM32_LOADER_GUI

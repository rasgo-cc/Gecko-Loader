#ifdef EFM32_LOADER_GUI

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "geckoloader.h"
#include "helpdialog.h"
#include <QDebug>
#include <QSerialPortInfo>
#include <QFileDialog>
#include <QSettings>
#include <QSerialPort>
#include <QMessageBox>
#include <QTextStream>
#include <QRegularExpression>
#include <QScrollBar>
#include <QVariant>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //ui->lineASCII->hide();
    ui->textLog->setFont(QFont("Courier New", 9));

    loader = new GeckoLoader(this);
    serialPort = loader->serialPort();
    bleDiscoveryAgent = new QBluetoothDeviceDiscoveryAgent(this);
    _connected = false;

    readSettings();
    slotReloadSerialPorts();

    connect(loader, SIGNAL(output(QString)), this, SLOT(log(QString)));
    connect(serialPort, SIGNAL(readyRead()), this, SLOT(slotDataReady()));
    connect(ui->buttonScanBLE, SIGNAL(clicked(bool)), this, SLOT(slotScanBLE()));
    connect(ui->buttonConnectBLE, SIGNAL(clicked(bool)), this, SLOT(slotConnectBLE()));
    connect(ui->buttonHelp, SIGNAL(clicked()), this, SLOT(slotHelp()));
    connect(ui->buttonReload, SIGNAL(clicked()), this, SLOT(slotReloadSerialPorts()));
    connect(ui->buttonBrowse, SIGNAL(clicked()), this, SLOT(slotBrowse()));
    connect(ui->buttonUpload, SIGNAL(clicked()), this, SLOT(slotUpload()));
    connect(ui->buttonConnect, SIGNAL(clicked()), this, SLOT(slotConnect()));
    connect(ui->comboTransport, SIGNAL(currentIndexChanged(int)), this, SLOT(slotTransport()));

    connect(ui->lineASCII, SIGNAL(returnPressed()), this, SLOT(slotSendASCII()));

    connect(bleDiscoveryAgent, SIGNAL(deviceDiscovered(QBluetoothDeviceInfo)),
            this, SLOT(slotDeviceDiscovered(QBluetoothDeviceInfo)));
    connect(bleDiscoveryAgent, SIGNAL(error(QBluetoothDeviceDiscoveryAgent::Error)),
            this, SLOT(slotDeviceScanError(QBluetoothDeviceDiscoveryAgent::Error)));
    connect(bleDiscoveryAgent, SIGNAL(finished()),
            this, SLOT(slotDeviceScanFinished()));

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
    QString output = text;
    if(output.contains(QRegularExpression("\\d+\\s+\\/\\s+\\d+")))
    {
        output = output.remove('[');
        output = output.remove(']');
        output = output.remove(' ');
        QStringList sl = output.split('/');
        int progress = (int)(sl[0].toFloat() / sl[1].toFloat() * 100.0);
        ui->progressBar->setValue(progress);
    }
    else
    {
        ui->textLog->appendPlainText(text);
    }
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
    if(!_connected)
    {
        _connected = loader->open(ui->comboPort->currentText());
    }
    else
    {
        loader->close();
        _connected = false;
    }
    updateInterface();
}

void MainWindow::slotTransport()
{
    QString curTransportStr = ui->comboTransport->currentText().toLower();
    if(curTransportStr == "uart")
        loader->setTransport(GeckoLoader::TransportUART);
    else if(curTransportStr == "usb")
        loader->setTransport(GeckoLoader::TransportUSB);
    else if(curTransportStr == "ble")
        loader->setTransport(GeckoLoader::TransportBLE);

    updateInterface();
}


void MainWindow::slotScanBLE()
{
    log("BLE scanning...");
    ui->comboBLEDevices->clear();
    bleDiscoveryAgent->start();
}

void MainWindow::slotConnectBLE()
{
    QString name = ui->comboBLEDevices->currentText();
    QString addr = ui->comboBLEDevices->currentData().toString();
    loader->connectBle(addr);
}

void MainWindow::slotTestBLE()
{

}

void MainWindow::slotDeviceDiscovered(QBluetoothDeviceInfo info)
{
    log("BLE device discovered: " + info.name() + " " + info.address().toString());
    ui->comboBLEDevices->addItem(info.name(), QVariant(info.address().toString()));
}

void MainWindow::slotDeviceScanError(QBluetoothDeviceDiscoveryAgent::Error err)
{
    log("BLE scan error: " + bleDiscoveryAgent->errorString());
}

void MainWindow::slotDeviceScanFinished()
{
    log("BLE scan finished");
}

void MainWindow::slotUpload()
{
    ui->textLog->clear();
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
    {
        QByteArray line = serialPort->readLine();
        if(!line.isEmpty())
        {
            ui->textLog->insertPlainText(line);
            QScrollBar *sb = ui->textLog->verticalScrollBar();
            sb->setValue(sb->maximum());
//            QString lineStr = QString(line);
//            lineStr = lineStr.remove('\n');
//            lineStr = lineStr.remove('\r');
//            ui->textLog->appendPlainText(lineStr);
//            ui->textLog->scr
        }
        //ui->textLog->appendPlainText(serialPort->readAll());
    }
}

void MainWindow::updateInterface()
{
    ui->buttonConnect->setDisabled(ui->comboPort->count() == 0);
    ui->buttonUpload->setEnabled(_connected);

    if(_connected)
        ui->buttonConnect->setText(tr("Disconnect"));
    else
        ui->buttonConnect->setText(tr("Connect"));

    ui->comboPort->setEnabled(!_connected);

    bool transportIsUart = ui->comboTransport->currentText() == "UART";
    bool transportIsBLE = ui->comboTransport->currentText() == "BLE";
    ui->labelBootEn->setVisible(transportIsUart);
    ui->comboBootEnPol->setVisible(transportIsUart);
    ui->widgetBLE->setVisible(transportIsBLE);
    if(transportIsBLE)
    {
        ui->stackedWidget->setCurrentIndex(1);
    }
    else
    {
        ui->stackedWidget->setCurrentIndex(0);
    }
}

#endif //EFM32_LOADER_GUI

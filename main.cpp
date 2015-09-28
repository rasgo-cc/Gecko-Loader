#include "mainwindow.h"

#ifdef EFM32_LOADER_GUI
#include <QApplication>
#include <QCoreApplication>
#include <QStyle>
#include <QStyleFactory>
#else
#include <QCoreApplication>
#endif

#include <QSettings>
#include <QDebug>
#include <QTimer>

#include "clhandler.h"
#include <iostream>

extern QTextStream cout;

void print_usage()
{
    cout << "Usage\n";
    cout << "UART: efm32_loader <port_name> <file_path> uart <boot_pol>\n";
    cout << "USB:  efm32_loader <port_name> <file_path> usb\n";
    cout.flush();
}

int main(int argc, char *argv[])
{
#ifdef EFM32_LOADER_GUI
    QApplication a(argc, argv);
    QStyle *style = QStyleFactory::create("Fusion");
    a.setStyle(style);
#else
    QCoreApplication a(argc, argv);
#endif

    a.setOrganizationDomain("settings");
    a.setApplicationName("EFM32 Loader");

    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, a.applicationDirPath());
    QSettings::setDefaultFormat(QSettings::IniFormat);

    CLHandler handler;

#ifdef EFM32_LOADER_GUI
    MainWindow w;
    if(argc == 1)
    {
        //cout << "efm33_loader GUI\n";
        //cout.flush();
        w.show();
    }
    else {
#endif

    cout << "efm32_loader CLI\n";

    if(argc != 4 && argc != 5)
    {
        cout << "Wrong number of arguments.\n";
        print_usage();
        exit(EXIT_FAILURE);
    }

    handler.portName = QString(argv[1]);
    handler.filePath = QString(argv[2]);
    handler.transport = QString(argv[3]).toLower();
    if(handler.transport == "uart")
    {
        if(argc != 5)
        {
            cout << "Wrong number of arguments.\n";
            print_usage();
            exit(EXIT_FAILURE);
        }
        handler.bootPol = QString(argv[4]);

        if(handler.bootPol != "0" && handler.bootPol != "1")
        {
            cout << "ERROR: <boot_pol> must be 0 or 1\n";
            print_usage();
            exit(EXIT_FAILURE);
        }
    }

    QObject::connect(&handler, SIGNAL(done()), &a, SLOT(quit()));
    QTimer::singleShot(0, &handler, SLOT(run()));

#ifdef EFM32_LOADER_GUI
    }
#endif

    return a.exec();
}

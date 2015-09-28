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
    a.setApplicationName("efm32 Loader");

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

    if(argc != 4)
    {
        cout << "Wrong number of arguments.\n";
        cout << "Usage: efm32_loader <port_name> <file_path> <boot_pol>\n";
        cout.flush();
        exit(EXIT_FAILURE);
    }

    handler.portName = QString(argv[1]);
    handler.filePath = QString(argv[2]);
    handler.bootPol = QString(argv[3]);

    if(handler.bootPol != "0" && handler.bootPol != "1")
    {
        cout << "ERROR: <boot_pol> must be 0 or 1\n";
        cout << "Usage: efm32_loader <port_name> <file_path> <boot_pol>\n";
        cout.flush();
        exit(EXIT_FAILURE);
    }

    QObject::connect(&handler, SIGNAL(done()), &a, SLOT(quit()));
    QTimer::singleShot(0, &handler, SLOT(run()));

#ifdef EFM32_LOADER_GUI
    }
#endif

    return a.exec();
}

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

#include "mainwindow.h"

#ifdef EFM32_LOADER_GUI
#include <QApplication>
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

#ifdef EFM32_LOADER_GUI
    MainWindow w;
    if(argc == 1)
    {
        std::cout << "EFM32 Loader GUI" << std::endl;
        w.show();
    }
    else {
#endif

    CLHandler handler;

    if(argc != 4)
    {
        std::cout << "Wrong number of arguments." << std::endl;
        std::cout << "Usage: efm32_loader <port_name> <file_path> <boot_pol>" << std::endl;
        exit(EXIT_FAILURE);
    }

    handler.portName = QString(argv[1]);
    handler.filePath = QString(argv[2]);
    handler.bootPol = QString(argv[3]);

    if(handler.bootPol != "0" && handler.bootPol != "1")
    {
        std::cout << "ERROR: <boot_pol> must be 0 or 1" << std::endl;
        std::cout << "Usage: efm32_loader <port_name> <file_path> <boot_pol>" << std::endl;
        exit(EXIT_FAILURE);
    }

    QObject::connect(&handler, SIGNAL(done()), &a, SLOT(quit()));
    QTimer::singleShot(0, &handler, SLOT(run()));

#ifdef EFM32_LOADER_GUI
    }
#endif

    return a.exec();
}

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

#include "clhandler.h"
#include <QTextStream>

QTextStream cout(stdout, QIODevice::WriteOnly);

void CLHandler::run()
{
    connect(&loader, SIGNAL(output(QString)), this, SLOT(log(QString)));

    cout << "> Connecting to port " << portName;
    cout.flush();

    if(bootPol == "1")
        loader.setBootEnablePolarity(true);
    else
        loader.setBootEnablePolarity(false);
    if(loader.open(portName))
       loader.upload(filePath);

    cout << "\n";
    cout.flush();

    emit done();
}

void CLHandler::log(const QString &message)
{
    static bool firstPercent = true;
    if(message.contains("/"))
    {
        if(firstPercent)
        {
            cout << "\n";
            firstPercent = false;
        }
        cout << message << "\r";

    }
    else
        cout << "\n> " << message;

    cout.flush();
}

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

#ifndef CLHANDLER_H
#define CLHANDLER_H

#include <QObject>
#include "efm32loader.h"

class CLHandler : public QObject
{
    Q_OBJECT
public:
    QString portName;
    QString filePath;
    QString bootPol;

public slots:
    void run();

private slots:
    void log(const QString &message);

signals:
    void done();

private:
    EFM32Loader loader;

};

#endif // CLHANDLER_H

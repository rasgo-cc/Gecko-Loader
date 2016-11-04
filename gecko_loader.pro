#-------------------------------------------------
#
# Project created by QtCreator 2014-04-13T04:02:34
#
#-------------------------------------------------

QT      += core
QT      += gui

CONFIG += console

greaterThan(QT_MAJOR_VERSION, 4): QT += serialport widgets bluetooth

TARGET = gecko_loader
TEMPLATE = app

#DEFINES += QT_NO_DEBUG_OUTPUT
DEFINES += EFM32_LOADER_GUI EFM32LOADER_SERIAL=1 EFM32LOADER_BLE=1

SOURCES += main.cpp\
        mainwindow.cpp \
    xmodem.cpp \
    clhandler.cpp \
    helpdialog.cpp \
    geckoloader.cpp

HEADERS  += mainwindow.h \
    xmodem.h \
    clhandler.h \
    helpdialog.h \
    geckoloader.h

FORMS    += mainwindow.ui \
    helpdialog.ui

CONFIG(debug, debug|release) {
    DESTDIR = debug
} else {
    DESTDIR = release
}

OBJECTS_DIR = build/obj
MOC_DIR = build/moc
RCC_DIR = build/rcc
UI_DIR = build/ui

RESOURCES += \
    resources.qrc

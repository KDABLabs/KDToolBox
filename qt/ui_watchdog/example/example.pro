TEMPLATE = app
TARGET = ui_watchdog_example
INCLUDEPATH += .
INCLUDEPATH += ../

SOURCES += main.cpp
QT += widgets

HEADERS += $$PWD/../uiwatchdog.h

CONFIG += console

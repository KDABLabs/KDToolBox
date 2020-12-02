TEMPLATE = app
TARGET = example
INCLUDEPATH += .

SOURCES += main.cpp ../QmlStackTraceHelper.cpp
QT += qml-private quick
CONFIG += debug

RESOURCES += qrc.qrc

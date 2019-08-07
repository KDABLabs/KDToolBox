#-------------------------------------------------
#
# Project created by QtCreator 2017-01-17T06:48:10
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = listUpdateAlgorithm
TEMPLATE = app


SOURCES += main.cpp\
        MainWindow.cpp \
    tableModel.cpp

HEADERS  += MainWindow.h \
    tableModel.h \
    Data.h \
    ../UpdateableModel.h

FORMS    += MainWindow.ui

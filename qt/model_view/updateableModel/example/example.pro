TEMPLATE = app
QT       += core gui widgets
TARGET = listUpdateAlgorithm


SOURCES += main.cpp\
        MainWindow.cpp \
    tableModel.cpp

HEADERS  += MainWindow.h \
    tableModel.h \
    Data.h \
    ../UpdateableModel.h

FORMS    += MainWindow.ui

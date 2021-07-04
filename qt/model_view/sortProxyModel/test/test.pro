CONFIG += testcase
QT       += testlib

TARGET = tst_sortproxymodeltest
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

SOURCES += \
        tst_sortproxymodeltest.cpp \
        $${PWD}/../sortproxymodel.cpp \

HEADERS += \
        $${PWD}/../sortproxymodel.h \
        vectormodel.h

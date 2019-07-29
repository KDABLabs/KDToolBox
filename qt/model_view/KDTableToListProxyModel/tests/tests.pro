include(../common.pri)

TEMPLATE = app
TARGET = tst_kdtabletolistproxymodel
QT = core gui testlib
CONFIG += testcase

SOURCES += \
    tst_kdtabletolistproxymodel.cpp \

DEFINES +=\
    QT_NO_CAST_FROM_ASCII \
    QT_NO_CAST_TO_ASCII \

include(../common.pri)

TEMPLATE = app
TARGET = tst_kdfunctionalsortfilterproxymodel
QT = core gui testlib
CONFIG += testcase

contains(QT_CONFIG, c++14):CONFIG *= c++14
contains(QT_CONFIG, c++1z):CONFIG *= c++1z
contains(QT_CONFIG, c++2a):CONFIG *= c++2a

SOURCES += \
    tst_kdfunctionalsortfilterproxymodel.cpp \

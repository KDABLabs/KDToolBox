include(../../common.pri)

CONFIG += testcase
TARGET = tst_messagehandler
QT = core testlib
contains(QT_CONFIG, c++14):CONFIG *= c++14
contains(QT_CONFIG, c++1z):CONFIG *= c++1z
contains(QT_CONFIG, c++2a):CONFIG *= c++2a
SOURCES += tst_messagehandler.cpp

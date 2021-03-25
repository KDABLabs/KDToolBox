CONFIG += testcase
QT = core testlib
SOURCES = ../tst_toContainer.cpp
TARGET = tst_toContainer_cpplatest

CONFIG += c++17 strict_c++

# not 100% ideal as it looks at Qt's config rather than compiler capabilities,
# but qmake is qmake...
contains(QT_CONFIG, c++2a):CONFIG *= c++2a

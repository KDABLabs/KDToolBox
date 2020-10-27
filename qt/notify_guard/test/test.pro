QT += testlib

CONFIG += qt console warn_on depend_includepath testcase
CONFIG -= app_bundle

TEMPLATE = app

# Include Directories under test
INCLUDEPATH += ../src/ \

# Sources under test
SOURCES += \
        ../src/notifyguard.cpp \

# Headers under test
HEADERS += \
        ../src/notifyguard.h \

# Test sources
SOURCES +=  \
        tst_notifyguard.cpp

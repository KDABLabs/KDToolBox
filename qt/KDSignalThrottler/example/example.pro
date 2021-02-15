TEMPLATE = app

SOURCES += \
    main.cpp \
    ../src/KDSignalThrottler.cpp

HEADERS += \
    ../src/KDSignalThrottler.h

INCLUDEPATH += \
    ../src/

QT += widgets
CONFIG += c++14

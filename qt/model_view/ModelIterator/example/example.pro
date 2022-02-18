config_cpp20 {
    CONFIG += c++2a
} else {
    CONFIG += c++14
}


SOURCES += main.cpp \
    ../src/ModelIterator.cpp

HEADERS += \
    ../src/ModelIterator.h

TEMPLATE = lib

QT -= gui
CONFIG += sanitizer sanitize_address
SOURCES = asan_assert_fail_qt.cpp

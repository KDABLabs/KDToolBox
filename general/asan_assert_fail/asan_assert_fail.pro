TEMPLATE = lib

QT -= core gui
CONFIG -= qt
CONFIG += use_c_linker sanitizer sanitize_address
SOURCES = asan_assert_fail.c

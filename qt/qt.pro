TEMPLATE = subdirs
SUBDIRS += \
    model_view \
    qml \
    ui_watchdog \

linux {
    SUBDIRS += \
        asan_assert_fail_qt
}

TEMPLATE = subdirs
SUBDIRS += \
    eventfilter \
    KDSignalThrottler \
    KDSqlDatabaseTransaction \
    messagehandler \
    model_view \
    notify_guard \
    pointer_cast \
    qml \
    qt_fmt \
    qt_hasher \
    singleshot_connect \
    tabWindow \
    ui_watchdog \

versionAtLeast(QT_VERSION, 5.14):versionAtMost(QT_VERSION, 5.15):SUBDIRS += \
    stringtokenizer \

linux {
    SUBDIRS += \
        asan_assert_fail_qt
}

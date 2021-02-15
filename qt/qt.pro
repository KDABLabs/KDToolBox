TEMPLATE = subdirs
SUBDIRS += \
    eventfilter \
    KDSignalThrottler \
    messagehandler \
    model_view \
    notify_guard \
    qml \
    qt_hasher \
    singleshot_connect \
    tabWindow \
    ui_watchdog \

versionAtLeast(QT_VERSION, 5.14):SUBDIRS += \
    stringtokenizer \


TEMPLATE = subdirs
SUBDIRS += \
    eventfilter \
    messagehandler \
    model_view \
    qml \
    ui_watchdog \
    tabWindow \
    singleshot_connect \

versionAtLeast(QT_VERSION, 5.14):SUBDIRS += \
    stringtokenizer \


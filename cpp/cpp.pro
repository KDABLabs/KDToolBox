TEMPLATE = subdirs
SUBDIRS += \
    duplicatetracker \
    future-backports \

versionAtLeast(QT_VERSION, 5.15):SUBDIRS += \
    propagate_const \
    toContainer \

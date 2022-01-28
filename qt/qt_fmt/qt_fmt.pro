TEMPLATE = subdirs

exists($$PWD/fmt/.git) {
    SUBDIRS += \
        test
} else {
    message("Submodule fmt is not initialized, skipping")
}

OTHER_FILES += \
    README.md

TEMPLATE = subdirs

include(configtests.pri)

config_cpp20:message("Enabling C++20 tests")

SUBDIRS += \
    cpp \
    qt \
    general \

OTHER_FILES += \
    README.md

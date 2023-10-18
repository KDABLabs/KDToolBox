find_package(
    Qt${QT_VERSION_MAJOR}
    ${QT_REQUIRED_VERSION}
    CONFIG
    REQUIRED
    Core
    Gui
    Widgets
)

include_directories(../../src/)

set(debouncedSearch_SOURCES ../../src/KDSignalThrottler.cpp ../../src/KDSignalThrottler.h main.cpp)

add_executable(debouncedSearch ${debouncedSearch_SOURCES})
target_link_libraries(debouncedSearch PUBLIC Qt::Core Qt::Gui Qt::Widgets)

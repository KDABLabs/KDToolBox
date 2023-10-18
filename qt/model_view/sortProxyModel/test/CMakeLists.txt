find_package(
    Qt${QT_VERSION_MAJOR}
    ${QT_REQUIRED_VERSION}
    CONFIG
    REQUIRED
    Core
    Gui
    Test
)

add_definitions(-DQT_DEPRECATED_WARNINGS)
message(STATUS "console was found. Need to port to cmake")
set(tst_sortproxymodeltest_SOURCES ../sortproxymodel.cpp ../sortproxymodel.h tst_sortproxymodeltest.cpp vectormodel.h)

add_executable(tst_sortproxymodeltest ${tst_sortproxymodeltest_SOURCES})
target_link_libraries(tst_sortproxymodeltest PUBLIC Qt::Core Qt::Gui Qt::Test Qt::CorePrivate)

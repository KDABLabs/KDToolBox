find_package(
    Qt${QT_VERSION_MAJOR}
    ${QT_REQUIRED_VERSION}
    CONFIG
    REQUIRED
    Core
    Gui
    Test
)

include_directories(../src/)

set(tst_kdfunctionalsortfilterproxymodel_SOURCES
    ../src/KDFunctionalSortFilterProxyModel.cpp ../src/KDFunctionalSortFilterProxyModel.h
    tst_kdfunctionalsortfilterproxymodel.cpp
)

add_executable(tst_kdfunctionalsortfilterproxymodel ${tst_kdfunctionalsortfilterproxymodel_SOURCES})
target_link_libraries(tst_kdfunctionalsortfilterproxymodel PUBLIC Qt::Core Qt::Gui Qt::Test)

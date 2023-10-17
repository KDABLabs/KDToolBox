find_package(Qt${QT_VERSION_MAJOR} ${QT_REQUIRED_VERSION} CONFIG REQUIRED Core Sql Test)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(tst_KDSqlDatabaseTransaction_SOURCES
  tst_KDSqlDatabaseTransaction.cpp
)

add_executable(tst_KDSqlDatabaseTransaction ${tst_KDSqlDatabaseTransaction_SOURCES})
target_link_libraries(tst_KDSqlDatabaseTransaction
  PUBLIC
    Qt::Core
    Qt::Sql
    Qt::Test
)

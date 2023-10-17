set(asan_assert_fail_qt_SOURCES
  asan_assert_fail_qt.cpp
)

add_library(asan_assert_fail_qt SHARED ${asan_assert_fail_qt_SOURCES})
target_link_libraries(asan_assert_fail_qt
  PUBLIC
    Qt::Core
)

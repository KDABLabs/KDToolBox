find_package(Qt${QT_VERSION_MAJOR} ${QT_REQUIRED_VERSION} CONFIG REQUIRED Core Gui Widgets)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(example_SOURCES
  main.cpp
)

add_executable(example ${example_SOURCES})
target_link_libraries(example
  PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Widgets
)

find_package(Qt${QT_VERSION_MAJOR} ${QT_REQUIRED_VERSION} CONFIG REQUIRED Core Gui Widgets)

include_directories(
  ../src/
)
set(example_SOURCES
  ../src/messagehandler.cpp
  ../src/messagehandler.h
  main.cpp
)

add_executable(example_messagehandler ${example_SOURCES})
target_link_libraries(example_messagehandler
  PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Widgets
)

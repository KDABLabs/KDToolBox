find_package(Qt${QT_VERSION_MAJOR} ${QT_REQUIRED_VERSION} CONFIG REQUIRED Core Gui Widgets)

set(listUpdateAlgorithm_SOURCES
  ../UpdateableModel.h
  Data.h
  MainWindow.cpp
  MainWindow.h
  main.cpp
  tableModel.cpp
  tableModel.h
)

set(listUpdateAlgorithm_UIS
  MainWindow.ui
)
qt_wrap_ui(listUpdateAlgorithm_SOURCES ${listUpdateAlgorithm_UIS})

add_executable(listUpdateAlgorithm ${listUpdateAlgorithm_SOURCES})
target_link_libraries(listUpdateAlgorithm
  PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Widgets
)

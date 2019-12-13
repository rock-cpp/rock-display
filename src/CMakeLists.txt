set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")

rock_library(rock-display
SOURCES
    TaskModel.cpp
    TaskModelNotifier.cpp
    NameServiceModel.cpp
    PortItem.cpp
    TaskItem.cpp
    TypedItem.cpp
    Mainwindow.cpp
    Vizkit3dPluginRepository.cpp
    ConfigItem.cpp
    VisualizerAdapter.cpp
MOC
    TaskModel.hpp
        TaskModelNotifier.hpp
    NameServiceModel.hpp
    Mainwindow.hpp
    Types.hpp
    VisualizerAdapter.hpp
HEADERS
UI
    task_inspector_window.ui
DEPS_PKGCONFIG
    orocos_cpp
    Qt5Core
    Qt5Gui
    Qt5Widgets
    lib_config
    vizkit3d
    rock_widget_collection
    base-lib
    tinyxml
)
add_definitions(-DBASE_LOG_INFO)

rock_executable(rock-display_bin Main.cpp
DEPS
    rock-display
DEPS_PKGCONFIG
    orocos_cpp
    Qt5Core
    Qt5Gui
    Qt5Widgets
UI
    task_inspector_window.ui
)

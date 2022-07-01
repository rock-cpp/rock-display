rock_find_qt5(REQUIRED Gui Core Widgets)


rock_library(rock-display
SOURCES
    TaskModel.cpp
    TaskModelNotifier.cpp
    NameServiceModel.cpp
    NameServiceItemDelegate.cpp
    ConfigItemHandler.cpp
    PortItem.cpp
    TaskItem.cpp
    TypedItem.cpp
    Mainwindow.cpp
    Vizkit3dPluginRepository.cpp
    ConfigItem.cpp
    VisualizerAdapter.cpp
    configitemhandlers/EnumHandler.cpp
    ConfigItemHandlerRepository.cpp
MOC
    TaskModel.hpp
    TaskModelNotifier.hpp
    NameServiceModel.hpp
    NameServiceItemDelegate.hpp
    ConfigItemHandler.hpp
    Mainwindow.hpp
    Types.hpp
    VisualizerAdapter.hpp
    configitemhandlers/EnumHandler.hpp
    ConfigItemHandlerRepository.hpp
HEADERS
UI
    task_inspector_window.ui
DEPS_PKGCONFIG
    orocos_cpp
    lib_config
    vizkit3d
    rock_widget_collection
    base-lib
    tinyxml
LIBS Qt5::Core
     Qt5::Gui
     Qt5::Widgets
)
add_definitions(-DBASE_LOG_INFO)

rock_executable(rock-display_bin Main.cpp
DEPS
    rock-display
DEPS_PKGCONFIG
    orocos_cpp
LIBS
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
UI
    task_inspector_window.ui
)

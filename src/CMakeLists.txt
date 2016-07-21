rock_library(rock-display
SOURCES
    TaskModel.cpp
    TaskItem.cpp
    PortItem.cpp
    TypedItem.cpp
    RDItemDelegate.cpp
MOC
    ConfigItem.cpp
    TaskModel.hpp
    RDItemDelegate.hpp
    Types.hpp
HEADERS
DEPS_PKGCONFIG
    orocos_cpp
    QtCore
    QtGui
    lib_config
)

rock_executable(rock-display_bin Main.cpp
DEPS
    rock-display
DEPS_PKGCONFIG
    orocos_cpp
    QtCore
    QtGui
UI
    task_inspector_window.ui
)

rock_executable(list_tasks List.cpp
DEPS_PKGCONFIG
    orocos_cpp
)

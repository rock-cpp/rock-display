rock_library(rock-display
SOURCES
    TaskModel.cpp
    TaskItem.cpp
    PortItem.cpp
    TypedItem.cpp
MOC
    TaskModel.hpp
    RDItemDelegate.hpp
HEADERS
DEPS_PKGCONFIG
    orocos_cpp
    QtCore
    QtGui
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

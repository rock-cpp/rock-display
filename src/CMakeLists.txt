rock_library(rock-display
SOURCES 
    TaskModel.cpp
MOC 
    TaskModel.hpp
HEADERS 
DEPS_PKGCONFIG 
    orocos_cpp
    QtCore 
    QtGui
)

rock_executable(rock-display_bin Main.cpp TaskModel.cpp
DEPS
    rock-display
MOC TaskModel.hpp
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


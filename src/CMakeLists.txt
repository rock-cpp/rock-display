rock_find_qt5(REQUIRED Gui Core Widgets)

find_package(PkgConfig)

pkg_check_modules(TRANSFORMER_TYPEKIT IMPORTED_TARGET transformer-typekit-gnulinux)

rock_library(rock-display
SOURCES
    TaskModel.cpp
    TaskModelNotifier.cpp
    NameServiceModel.cpp
    NameServiceItemDelegate.cpp
    ConfigItemHandler.cpp
    PortItem.cpp
    PropertyItem.cpp
    TaskItem.cpp
    TypedItem.cpp
    Mainwindow.cpp
    ConfigItem.cpp
    VisualizerAdapter.cpp
    configuration.cpp
    configitemhandlers/EnumHandler.cpp
    configitemhandlers/ContainerHandler.cpp
    configitemhandlers/StdStringHandler.cpp
    configitemhandlers/ImageCtxMenuHandler.cpp
    configitemhandlers/BinaryCtxMenuHandler.cpp
    ConfigItemHandlerRepository.cpp
    vizplugins/imageviewplugin.cpp
    vizplugins/virtualjoystickplugin.cpp
    vizplugins/vizkitplugin.cpp
    vizplugins/vizkit3dplugins.cpp
    vizplugins/Vizkit3dPluginRepository.cpp
    ConfigurationSelectDialog.cpp
MOC
    TaskModel.hpp
    TaskModelNotifier.hpp
    NameServiceModel.hpp
    NameServiceItemDelegate.hpp
    ConfigItemHandler.hpp
    Mainwindow.hpp
    Types.hpp
    VisualizerAdapter.hpp
    VizHandle.hpp
    PluginHandle.hpp
    configitemhandlers/EnumHandler.hpp
    configitemhandlers/ContainerHandler.hpp
    configitemhandlers/StdStringHandler.hpp
    configitemhandlers/ImageCtxMenuHandler.hpp
    configitemhandlers/BinaryCtxMenuHandler.hpp
    ConfigItemHandlerRepository.hpp
    vizplugins/imageviewplugin.hpp
    vizplugins/imageviewplugin_p.hpp
    vizplugins/virtualjoystickplugin.hpp
    vizplugins/virtualjoystickplugin_p.hpp
    vizplugins/vizkitplugin.hpp
    vizplugins/vizkitplugin_p.hpp
    vizplugins/Vizkit3dPluginRepository.hpp
    vizplugins/vizkit3dplugins.hpp
    vizplugins/vizkit3dplugins_p.hpp
    ConfigurationSelectDialog.hpp
HEADERS
UI
    task_inspector_window.ui
    ConfigurationSelectDialog.ui
DEPS_PKGCONFIG
    orocos_cpp
    lib_config
    vizkit3d
    rock_widget_collection
    base-lib
    tinyxml
    frame_helper
LIBS Qt5::Core
     Qt5::Gui
     Qt5::Widgets
)

if(TRANSFORMER_TYPEKIT_FOUND)
    target_compile_definitions(rock-display PRIVATE HAVE_TRANSFORMER_TYPEKIT=1)
    target_include_directories(rock-display PRIVATE ${TRANSFORMER_TYPEKIT_INCLUDE_DIRS})
    target_link_libraries(rock-display ${TRANSFORMER_TYPEKIT_LINK_LIBRARIES})
    message(STATUS "Using transformer typekit for transformer types")
endif()

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
    ConfigurationSelectDialog.ui
)

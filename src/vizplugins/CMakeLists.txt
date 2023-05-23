
rock_find_cmake(Boost REQUIRED COMPONENTS regex)

rock_library(rockdisplay_vizkitplugin SHARED
    SOURCES
    vizkitplugin.cpp
    MOC
    vizkitplugin.hpp
    HEADERS
    vizkitplugin.hpp
    )

rock_library_common(rockdisplay_base_types_plugins SHARED
    SOURCES
    artificialhorizonplugin.cpp
    imageviewplugin.cpp
    orientationviewplugin.cpp
    rangeviewplugin.cpp
    sonardisplayplugin.cpp
    sonarviewplugin.cpp
    sonarwidgetplugin.cpp
    streamalignerwidgetplugin.cpp
    virtualjoystickplugin.cpp
    designer/pluginmanager.cpp
    base_types_pluginfactory.cpp

    MOC
    artificialhorizonplugin.hpp
    artificialhorizonplugin_p.hpp
    imageviewplugin.hpp
    imageviewplugin_p.hpp
    orientationviewplugin.hpp
    orientationviewplugin_p.hpp
    rangeviewplugin.hpp
    rangeviewplugin_p.hpp
    sonardisplayplugin.hpp
    sonardisplayplugin_p.hpp
    sonarviewplugin.hpp
    sonarviewplugin_p.hpp
    sonarwidgetplugin.hpp
    sonarwidgetplugin_p.hpp
    streamalignerwidgetplugin.hpp
    streamalignerwidgetplugin_p.hpp
    virtualjoystickplugin.hpp
    virtualjoystickplugin_p.hpp
    designer/pluginmanager_p.h
    base_types_pluginfactory.hpp

    DEPS_PKGCONFIG base-types
    LIBS Qt5::Widgets Qt5::Gui
    DEPS rockdisplay_vizkitplugin
    )

install(TARGETS rockdisplay_base_types_plugins
        LIBRARY DESTINATION lib/qt/rockdisplay)
install(FILES ${rockdisplay_base_types_plugins_HEADERS}
        DESTINATION include/${PROJECT_NAME})

rock_library_common(rockdisplay_plot2dplugin SHARED
    SOURCES
    plot2dplugin.cpp
    designer/pluginmanager.cpp
    plot2d_pluginfactory.cpp

    MOC
    plot2dplugin.hpp
    plot2dplugin_p.hpp
    designer/pluginmanager_p.h
    plot2d_pluginfactory.hpp

    DEPS_PKGCONFIG base-types typelib
    DEPS rockdisplay_vizkitplugin
    LIBS Qt5::Widgets Qt5::Gui
    )

install(TARGETS rockdisplay_plot2dplugin
        LIBRARY DESTINATION lib/qt/rockdisplay)
install(FILES ${rockdisplay_plot2dplugin_HEADERS}
        DESTINATION include/${PROJECT_NAME})

set(TRANSFORMER_LIBS)
if(TRANSFORMER_TYPEKIT_FOUND)
    set(TRANSFORMER_LIBS transformer-typekit-gnulinux orocos_cpp)
endif()

rock_library_common(rockdisplay_vizkit3dplugin SHARED
    SOURCES
    vizkit3dplugins.cpp
    Vizkit3dPluginRepository.cpp
    vizkit3d_pluginfactory.cpp

    MOC
    vizkit3dplugins.hpp
    vizkit3dplugins_p.hpp
    vizkit3d_pluginfactory.hpp

    DEPS_PKGCONFIG base-types vizkit3d-qt5 typelib
    ${TRANSFORMER_LIBS}
    DEPS rockdisplay_vizkitplugin
    LIBS Qt5::Widgets Qt5::Gui ${Boost_REGEX_LIBRARY}
    )

if(TRANSFORMER_TYPEKIT_FOUND)
    target_compile_definitions(rockdisplay_vizkit3dplugin PRIVATE HAVE_TRANSFORMER_TYPEKIT=1)
    message(STATUS "Using transformer typekit for transformer types")
endif()

install(TARGETS rockdisplay_vizkit3dplugin
        LIBRARY DESTINATION lib/qt/rockdisplay)
install(FILES ${rockdisplay_vizkit3dplugin_HEADERS}
        DESTINATION include/${PROJECT_NAME})

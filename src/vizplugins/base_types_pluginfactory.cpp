
#include "base_types_pluginfactory.hpp"

#include "artificialhorizonplugin.hpp"
#include "imageviewplugin.hpp"
#include "orientationviewplugin.hpp"
#include "rangeviewplugin.hpp"
#include "sonardisplayplugin.hpp"
#include "sonarviewplugin.hpp"
#include "sonarwidgetplugin.hpp"
#include "streamalignerwidgetplugin.hpp"
#include "virtualjoystickplugin.hpp"
//#include "plot2dplugin.hpp"
//#include "vizkit3dplugins.hpp"

std::vector<rockdisplay::vizkitplugin::Plugin *> rockdisplay::base_types::PluginFactory::createPlugins()
{
    std::vector<rockdisplay::vizkitplugin::Plugin *> plugins;
    plugins.push_back(new rock_display::ArtificialHorizonPlugin);
    plugins.push_back(new rock_display::ImageViewPlugin);
    plugins.push_back(new rock_display::OrientationViewPlugin);
    plugins.push_back(new rock_display::RangeViewPlugin);
    plugins.push_back(new rock_display::SonarDisplayPlugin);
    plugins.push_back(new rock_display::SonarViewPlugin);
    plugins.push_back(new rock_display::SonarWidgetPlugin);
    plugins.push_back(new rock_display::StreamAlignerWidgetPlugin);
    plugins.push_back(new rock_display::VirtualJoystickPlugin);
    //plugins.push_back(new rock_display::Plot2dPlugin);
    //plugins.push_back(new rock_display::Vizkit3DPlugins);
    return plugins;
}


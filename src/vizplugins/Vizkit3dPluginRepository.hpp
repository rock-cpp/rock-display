#pragma once

#include <map>
#include <vizkit3d/Vizkit3DPlugin.hpp>
#include <string>
#include <algorithm>
#include <QObject>

QT_BEGIN_NAMESPACE
class QWidget;
QT_END_NAMESPACE

namespace Typelib
{
    class Registry;
}

class Vizkit3dPluginHandle
{
public:
    std::string pluginName;
    vizkit3d::VizkitPluginFactory *factory;
    std::string libararyName;
    std::string typeName; //this is a dotted type name, global scoped, without :: prefix
    QMetaMethod method;
    Vizkit3dPluginHandle(std::string const &pluginName);
    virtual ~Vizkit3dPluginHandle() = default;
};

class Vizkit3dPluginRepository
{
    std::vector<Vizkit3dPluginHandle*> empty;
    
    std::map<std::string, std::vector<Vizkit3dPluginHandle*> > typeToPlugins;
    std::vector<Vizkit3dPluginHandle*> datalessPlugins;
public:
    Vizkit3dPluginRepository(QStringList &plugins);
    
    const std::vector<Vizkit3dPluginHandle*> &getPluginsForType(const std::string &type, const Typelib::Registry* registry = NULL);
    
    const std::vector<Vizkit3dPluginHandle*> getAllAvailablePlugins();
};

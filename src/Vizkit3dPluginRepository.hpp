#pragma once

#include <map>
#include <vizkit3d/Vizkit3DPlugin.hpp>

#include <QObject>

class VizHandle
{
public:
    QMetaMethod method;
    QObject *plugin;
};

class PluginHandle
{
public:
    vizkit3d::VizkitPluginFactory *factory;
    std::string libararyName;
    std::string pluginName;
    std::string typeName;
    QMetaMethod method;
};


class Vizkit3dPluginRepository
{
    std::vector<PluginHandle> empty;
    
    std::map<std::string, std::vector<PluginHandle> > typeToPlugins;
public:
    Vizkit3dPluginRepository(QStringList &plugins);
    
    VizHandle getNewVizHandle(const PluginHandle &handle);
    
    const std::vector<PluginHandle> &getPluginsForType(const std::string &type);
};


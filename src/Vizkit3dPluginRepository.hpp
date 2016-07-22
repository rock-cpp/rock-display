#pragma once

#include <map>
#include <vizkit3d/Vizkit3DPlugin.hpp>

class PluginHandle
{
public:
    vizkit3d::VizkitPluginFactory *factory;
    std::string libararyName;
    std::string pluginName;
    std::string typeName;
};

class Vizkit3dPluginRepository
{
    std::vector<PluginHandle> empty;
    
    std::map<std::string, std::vector<PluginHandle> > typeToPlugins;
public:
    Vizkit3dPluginRepository(QStringList &plugins);
    
    const std::vector<PluginHandle> &getPluginsForType(const std::string &type);
};


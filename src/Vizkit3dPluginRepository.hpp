#pragma once

#include <map>
#include <vizkit3d/Vizkit3DPlugin.hpp>
#include <string>
#include <algorithm>

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

/** A functor that can be used with std::sort to sort to sort PluginHandles */
struct PluginHandleSortByPluginName
{
    bool ascending;
    /** @p ascending If true sort ascending, otherwise sort descending*/
    PluginHandleSortByPluginName(bool ascending = true) : ascending(ascending) {}
    
    inline bool operator() (const PluginHandle& a, const PluginHandle& b)
    {
        if(ascending)
            return std::lexicographical_compare(a.pluginName.begin(), a.pluginName.end(),
                                                b.pluginName.begin(), b.pluginName.end());
        else
            return std::lexicographical_compare(b.pluginName.begin(), b.pluginName.end(),
                                                a.pluginName.begin(), a.pluginName.end());
    }
};

class Vizkit3dPluginRepository
{
    std::vector<PluginHandle> empty;
    
    std::map<std::string, std::vector<PluginHandle> > typeToPlugins;
public:
    Vizkit3dPluginRepository(QStringList &plugins);
    
    VizHandle getNewVizHandle(const PluginHandle &handle);
    
    const std::vector<PluginHandle> &getPluginsForType(const std::string &type);
    
    const std::vector<PluginHandle> getAllAvailablePlugins();
};
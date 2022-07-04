#pragma once

#include <map>
#include <vizkit3d/Vizkit3DPlugin.hpp>
#include <string>
#include <algorithm>
#include <typelib/registry.hh>
#include <QObject>
#include <rtt/base/DataSourceBase.hpp>

QT_BEGIN_NAMESPACE
class QWidget;
QT_END_NAMESPACE

class VizHandle
{
public:
    virtual ~VizHandle() {}
    virtual void updateVisualizer(RTT::base::DataSourceBase::shared_ptr data) = 0;
    virtual QObject *getVizkit3dPluginObject() = 0;
    virtual QWidget *getStandaloneWidget() = 0;
};

class Vizkit3dVizHandle : public VizHandle
{
public:
    QMetaMethod method;
    QObject *plugin;
    virtual void updateVisualizer(RTT::base::DataSourceBase::shared_ptr data) override;
    virtual QObject *getVizkit3dPluginObject() override { return plugin; }
    virtual QWidget *getStandaloneWidget() override { return nullptr; }
};

class PluginHandle
{
public:
    std::string pluginName;
    virtual ~PluginHandle() {}
    virtual VizHandle *createViz() const = 0;
};

class Vizkit3dPluginHandle : public PluginHandle
{
public:
    vizkit3d::VizkitPluginFactory *factory;
    std::string libararyName;
    std::string typeName;
    QMetaMethod method;
    virtual VizHandle *createViz() const override;
};

/** A functor that can be used with std::sort to sort to sort PluginHandles */
struct PluginHandleSortByPluginName
{
    bool ascending;
    /** @p ascending If true sort ascending, otherwise sort descending*/
    PluginHandleSortByPluginName(bool ascending = true) : ascending(ascending) {}
    
    inline bool operator() (const PluginHandle* a, const PluginHandle* b)
    {
        if(ascending)
            return std::lexicographical_compare(a->pluginName.begin(), a->pluginName.end(),
                                                b->pluginName.begin(), b->pluginName.end());
        else
            return std::lexicographical_compare(b->pluginName.begin(), b->pluginName.end(),
                                                a->pluginName.begin(), a->pluginName.end());
    }
};

class Vizkit3dPluginRepository
{
    std::vector<Vizkit3dPluginHandle> empty;
    
    std::map<std::string, std::vector<Vizkit3dPluginHandle> > typeToPlugins;
public:
    Vizkit3dPluginRepository(QStringList &plugins);
    
    const std::vector<Vizkit3dPluginHandle> &getPluginsForType(const std::string &type, const Typelib::Registry* registry = NULL);
    
    const std::vector<Vizkit3dPluginHandle*> getAllAvailablePlugins();
};

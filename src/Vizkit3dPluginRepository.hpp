#pragma once

#include <map>
#include <vizkit3d/Vizkit3DPlugin.hpp>
#include <string>
#include <algorithm>
#include <QObject>
#include <rtt/base/DataSourceBase.hpp>
#include "VizHandle.hpp"
#include "PluginHandle.hpp"

namespace Typelib
{
    class Registry;
    class Type;
}

class Vizkit3dVizHandle : public VizHandle
{
    Q_OBJECT
public:
    QMetaMethod method;
    QObject *plugin;
    virtual QObject *getVizkit3dPluginObject() override { return plugin; }
    virtual QWidget *getStandaloneWidget() override { return nullptr; }
public slots:
    virtual void updateVisualizer(void const *data, RTT::base::DataSourceBase::shared_ptr base_sample) override;
};

class Vizkit3dPluginHandle : public PluginHandle
{
public:
    vizkit3d::VizkitPluginFactory *factory;
    std::string libararyName;
    std::string typeName; //this is a dotted type name, global scoped, without :: prefix
    QMetaMethod method;
    Vizkit3dPluginHandle(std::string const &pluginName);
    virtual VizHandle *createViz() const override;
    virtual bool probe(Typelib::Type const &type, const Typelib::Registry* registry = NULL) const override;
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
    std::vector<Vizkit3dPluginHandle*> empty;
    
    std::map<std::string, std::vector<Vizkit3dPluginHandle*> > typeToPlugins;
public:
    Vizkit3dPluginRepository(QStringList &plugins);
    
    const std::vector<Vizkit3dPluginHandle*> &getPluginsForType(const std::string &type, const Typelib::Registry* registry = NULL);
    
    const std::vector<Vizkit3dPluginHandle*> getAllAvailablePlugins();
};

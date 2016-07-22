#pragma once

#include <QStandardItem>
#include "TypedItem.hpp"
#include "Types.hpp"
#include "ConfigItem.hpp"
#include "Vizkit3dPluginRepository.hpp"

namespace RTT
{
    namespace base
    {
        class OutputPortInterface;
        class InputPortInterface;
    }
}

class PortItem
{
protected:
    std::shared_ptr<ItemBase> item;
    TypedItem *nameItem;
    TypedItem *valueItem;

public:
    PortItem(const std::string &name);
    QList<QStandardItem *> getRow();
};

class PortHandle;

class OutputPortItem : public PortItem
{
    std::vector<VizHandle> activeVizualizer;
    PortHandle *handle;
    RTT::base::InputPortInterface *reader;
    std::vector<PluginHandle> activePlugins;
    
public:
    OutputPortItem(RTT::base::OutputPortInterface* port);
    bool updataValue();
    
    const std::string &getType();
    
    void addPlugin(const VizHandle &handle);
};

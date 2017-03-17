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
        class PortInterface;
    }
}

class PortItem : public VisualizerAdapter
{
protected:
    std::shared_ptr<ItemBase> item;
    TypedItem *nameItem;
    TypedItem *valueItem;

public:
    PortItem(const std::string &name);
    virtual ~PortItem();
    QList<QStandardItem *> getRow();
    
    std::shared_ptr<ItemBase> getItemBase()
    {
        return item;
    }
};

class PortHandle;

class OutputPortItem : public PortItem
{
    PortHandle *handle;
    RTT::base::InputPortInterface *reader;
    
public:
    OutputPortItem(RTT::base::OutputPortInterface* port);
    virtual ~OutputPortItem();
    bool updataValue();
    
    void updateOutputPortInterface(RTT::base::OutputPortInterface* port);
    const std::string &getType();
    RTT::base::PortInterface* getPort();
};
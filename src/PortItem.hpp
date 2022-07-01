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
    std::string typeInfo;
    ConfigItemHandlerRepository *handlerrepo;

public:
    PortItem(const std::string &name, ConfigItemHandlerRepository *handlerrepo);
    virtual ~PortItem();
    QList<QStandardItem *> getRow();
    
    std::shared_ptr<ItemBase> getItemBase()
    {
        return item;
    }
    
    TypedItem *getNameItem()
    {
        return nameItem;
    }
    
    TypedItem *getValueItem()
    {
        return valueItem;
    }
    
    virtual bool hasVisualizers()
    {
        if (VisualizerAdapter::hasVisualizers() || (item && item->hasVisualizers()))
        {
            return true;
        }
        
        return false;
    }
};

class PortHandle;

class OutputPortItem : public PortItem
{
    PortHandle *handle;
    RTT::base::InputPortInterface *reader;
    
public:
    OutputPortItem(RTT::base::OutputPortInterface* port, ConfigItemHandlerRepository *handlerrepo);
    virtual ~OutputPortItem();
    bool updataValue();
    
    void updateOutputPortInterface(RTT::base::OutputPortInterface* port);
    const std::string &getType();
    RTT::base::PortInterface* getPort();
    
    void reset();
};

class InputPortItem : public PortItem
{
    PortHandle *handle;
    RTT::base::OutputPortInterface *writer;

public:
    InputPortItem(RTT::base::InputPortInterface* port, ConfigItemHandlerRepository *handlerrepo);
    virtual ~InputPortItem();
    bool updataValue();

    void updateInputPortInterface(RTT::base::InputPortInterface* port);
    const std::string &getType();
    RTT::base::PortInterface* getPort();

    void reset();
};

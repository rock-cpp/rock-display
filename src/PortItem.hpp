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

class PortItem
{
protected:
    std::shared_ptr<ItemBase> item;
    TypedItem *nameItem;
    TypedItem *valueItem;
    bool expanded;

public:
    PortItem(const std::string &name);
    virtual ~PortItem() {};
    QList<QStandardItem *> getRow();
    
    void setExpanded(bool expanded)
    {
        this->expanded = expanded;
    }
};

class PortHandle;

class OutputPortItem : public PortItem
{
    std::vector<VizHandle> activeVizualizer;
    PortHandle *handle;
    RTT::base::InputPortInterface *reader;
    
public:
    OutputPortItem(RTT::base::OutputPortInterface* port);
    bool updataValue();
    
    void updateOutputPortInterface(RTT::base::OutputPortInterface* port);
    const std::string &getType();
    RTT::base::PortInterface* getPort();
    
    void addPlugin(VizHandle &handle);
};

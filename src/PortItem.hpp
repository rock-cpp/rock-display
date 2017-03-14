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
    std::map<std::string, VizHandle> waitingVizualizer;

public:
    void addPlugin(std::pair<std::string, VizHandle> handle);
    PortItem(const std::string &name);
    virtual ~PortItem();
    QList<QStandardItem *> getRow();
    
    std::shared_ptr<ItemBase> getItemBase()
    {
        return item;
    }
    
    bool hasVisualizer(const std::string &name);
    bool removeVisualizer(QObject *plugin);
    QObject *getVisualizer(const std::string &name);
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
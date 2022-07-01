#pragma once
#include <QStandardItem>
#include "PortItem.hpp"
#include "TypedItem.hpp"
#include "Types.hpp"

namespace RTT
{
    namespace corba
    {
        class TaskContextProxy;
    }
}

class TaskItem
{   
    
private:
    RTT::corba::TaskContextProxy *task;
    TypedItem nameItem;
    TypedItem statusItem;
    bool refreshOutputPorts;
    bool refreshInputPorts;
    TypedItem inputPorts;
    TypedItem outputPorts;
    TypedItem properties;
    std::map<std::string, PortItem *> ports;
    std::map<std::string, std::shared_ptr<ItemBase>> propertyMap;
    bool stateChanged;
    ConfigItemHandlerRepository *handlerrepo;
    
public:
    TaskItem(RTT::corba::TaskContextProxy* _task, ConfigItemHandlerRepository *handlerrepo);
    virtual ~TaskItem();
    bool updateState();
    bool updatePorts(bool hasVisualizers=false);
    bool updateProperties();
    
    void setRefreshPorts(bool refresh=true)
    {
        this->refreshOutputPorts = refresh;
        this->refreshInputPorts = refresh;
    }

    bool update();
    bool hasVisualizers();
    
    void updateTaskContext(RTT::corba::TaskContextProxy* _task)
    {
        this->task = _task;
    }

    RTT::corba::TaskContextProxy* getTaskContext();

    QList<QStandardItem *> getRow();
    QStandardItem &getInputPorts()
    {
        return inputPorts;
    }
    
    QStandardItem &getOutputPorts()
    {
        return outputPorts;
    }

    QModelIndex updateLeft();
    QModelIndex updateRight();
    
    std::map<std::string, PortItem *> &getPorts()
    {
        return this->ports;
    }
    
    void reset();
};
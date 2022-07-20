#pragma once
#include "PortItem.hpp"
#include "TypedItem.hpp"
#include "Types.hpp"

QT_BEGIN_NAMESPACE
class QStandardItem;
QT_END_NAMESPACE

namespace RTT
{
    namespace corba
    {
        class TaskContextProxy;
    }
}

class PropertyItem;

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
    std::map<std::string, PropertyItem *> propertyMap;
    bool stateChanged;
    ConfigItemHandlerRepository *handlerrepo;
    
public:
    TaskItem(RTT::corba::TaskContextProxy* _task, ConfigItemHandlerRepository *handlerrepo);
    virtual ~TaskItem();
    bool updateState(bool updateUI = true);
    void updatePorts(bool hasVisualizers=false, bool updateUI = true);
    void updateProperties(bool updateUI = true);
    
    void setRefreshPorts(bool refresh=true)
    {
        this->refreshOutputPorts = refresh;
        this->refreshInputPorts = refresh;
    }

    void update(bool updateUI = true);
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
    
    std::map<std::string, PropertyItem *> &getProperties()
    {
        return this->propertyMap;
    }

    void reset();
};

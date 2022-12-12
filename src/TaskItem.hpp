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
    std::mutex taskMutex;
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
    void synchronizeTask();
    
public:
    TaskItem(RTT::corba::TaskContextProxy* _task, ConfigItemHandlerRepository *handlerrepo);
    virtual ~TaskItem();
    bool updateState();
    void updatePorts(bool hasVisualizers=false, bool handleOldData = false);
    void updateProperties();
    
    void setRefreshPorts(bool refresh=true)
    {
        this->refreshOutputPorts = refresh;
        this->refreshInputPorts = refresh;
    }

    /*
     * @param updateUI       set to true if this call should and is allowed to change UI.
     *                       if set to false, this will not touch any QStandardItem.
     * @param handleOldData  set to true to bypass logic checking for changed data.
     */
    void update(bool handleOldData = false);
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

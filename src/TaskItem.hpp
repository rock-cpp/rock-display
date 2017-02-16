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

    QStandardItem inputPorts;
    QStandardItem outputPorts;
    QStandardItem properties;
    std::map<std::string, PortItem *> ports;
    std::map<std::string, std::shared_ptr<ItemBase>> propertyMap;
    
    std::string stateLbl;
    
public:
    TaskItem(RTT::corba::TaskContextProxy* _task);
    virtual ~TaskItem();
    bool updateState();
    bool updatePorts();
    
    bool refreshPorts;

    bool update();
    std::string getStatusLbl()
    {
        return stateLbl;
    }
    
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
};
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
    TypedItem nameItem;
    TypedItem statusItem;

    QStandardItem inputPorts;
    QStandardItem outputPorts;
    std::map<std::string, PortItem *> ports;
    RTT::corba::TaskContextProxy *task;

public:
    TaskItem(RTT::corba::TaskContextProxy* _task);
    void updateState();
    bool updatePorts();
    bool clearPorts();

    bool update();
    
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
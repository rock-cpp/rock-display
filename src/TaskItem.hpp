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

    bool update();

    RTT::corba::TaskContextProxy* getTaskContext();

    QList<QStandardItem *> getRow();

    QModelIndex updateLeft();
    QModelIndex updateRight();
};
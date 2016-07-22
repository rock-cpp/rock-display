#pragma once
#include <QStandardItem>
#include "PortItem.hpp"
#include "TypedItem.hpp"
#include "Types.hpp"

namespace RTT
{
    class TaskContext;
}

class TaskItem
{
private:
    TypedItem nameItem;
    TypedItem statusItem;

    QStandardItem inputPorts;
    QStandardItem outputPorts;
    std::map<std::string, PortItem *> ports;
    RTT::TaskContext *task;

public:
    TaskItem(RTT::TaskContext* _task);
    void updateState();
    bool updatePorts();

    bool update();

    RTT::TaskContext* getTaskContext();

    QList<QStandardItem *> getRow();

    QModelIndex updateLeft();
    QModelIndex updateRight();
};

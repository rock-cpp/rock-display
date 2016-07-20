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
public:
    TaskItem();
    void updateState(RTT::TaskContext* task);
    bool updatePorts(RTT::TaskContext* task);

    bool update(RTT::TaskContext *tk);

    QList<QStandardItem *> getRow();

    QModelIndex updateLeft();
    QModelIndex updateRight();
};

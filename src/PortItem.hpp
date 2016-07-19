#pragma once

#include <QStandardItem>

namespace RTT
{
    namespace base
    {
        class OutputPortInterface;
        class InputPortInterface;
    }
}

class PortItem
{
protected:
    QStandardItem nameItem;
    QStandardItem valueItem;

public:
    PortItem(const std::string &name);
    QList<QStandardItem *> getRow();
};


class OutputPortItem : public PortItem
{
    QStandardItem nameItem;
    QStandardItem valueItem;
    
    RTT::base::InputPortInterface *reader;

public:
    OutputPortItem(RTT::base::OutputPortInterface* port);
    bool updataValue();
};

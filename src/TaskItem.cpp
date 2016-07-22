#include <rtt/TaskContext.hpp>
#include "TaskItem.hpp"

TaskItem::TaskItem(RTT::TaskContext* _task) : task(_task), nameItem(ItemType::TASK), statusItem(ItemType::TASK)
{
    inputPorts.setText("InputPorts");
    outputPorts.setText("OutputPorts");
    nameItem.appendRow(&inputPorts);
    nameItem.appendRow(&outputPorts);
}

bool TaskItem::update()
{
    if(nameItem.text().isEmpty())
    {
        try {
            nameItem.setText(task->getName().c_str());
        }
        catch (...)
        {
            return false;
        }
    }

    updateState();
    updatePorts();

    return true;
}

bool TaskItem::updatePorts()
{
    const RTT::DataFlowInterface *dfi = task->ports();
    std::vector<std::string> portNames = dfi->getPortNames();

    for(RTT::base::PortInterface *pi : dfi->getPorts())
    {
        const std::string portName(pi->getName());
        RTT::base::OutputPortInterface *outIf = dynamic_cast<RTT::base::OutputPortInterface *>(pi);
        auto it = ports.find(portName);
        PortItem *item = nullptr;
        if(it == ports.end())
        {
            if(outIf)
            {
                item = new OutputPortItem(outIf);
                outputPorts.appendRow(item->getRow());
            }
            else
            {
                item = new PortItem(pi->getName());
                ports.insert(std::make_pair(portName, item));
                inputPorts.appendRow(item->getRow());
            }
            ports.insert(std::make_pair(portName, item));
        }
        else
        {
            item = it->second;
        }

        if(outIf)
        {
            OutputPortItem *outIt = static_cast<OutputPortItem *> (item);
            outIt->updataValue();
        }
    }

    return true;
}

void TaskItem::updateState()
{
    QString stateString;
    RTT::base::TaskCore::TaskState state = task->getTaskState();
    switch(state)
    {
        case RTT::base::TaskCore::Exception:
            stateString = "Exception";
            break;
        case RTT::base::TaskCore::FatalError:
            stateString = "FatalError";
            break;
        case RTT::base::TaskCore::Init:
            stateString = "Init";
            break;
        case RTT::base::TaskCore::PreOperational:
            stateString = "PreOperational";
            break;
        case RTT::base::TaskCore::Running:
            stateString = "Running";
            break;
        case RTT::base::TaskCore::RunTimeError:
            stateString = "RunTimeError";
            break;
        case RTT::base::TaskCore::Stopped:
            stateString = "Stopped";
            break;
    }
    statusItem.setText(stateString);

}

QList< QStandardItem* > TaskItem::getRow()
{
    return {&nameItem, &statusItem};
}

QModelIndex TaskItem::updateLeft()
{
    return nameItem.index();
}

QModelIndex TaskItem::updateRight()
{
    return statusItem.index();
}

RTT::TaskContext* TaskItem::getTaskContext()
{
    return task;
}

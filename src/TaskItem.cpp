#include <rtt/transports/corba/TaskContextProxy.hpp>
#include "TaskItem.hpp"

TaskItem::TaskItem(RTT::corba::TaskContextProxy* _task)
    : task(_task),
      nameItem(ItemType::TASK),
      statusItem(ItemType::TASK),
      refreshPorts(false)
{
    inputPorts.setText("InputPorts");
    outputPorts.setText("OutputPorts");
    nameItem.appendRow(&inputPorts);
    nameItem.appendRow(&outputPorts);
    nameItem.setData(this);
    statusItem.setData(this);
}

TaskItem::~TaskItem()
{
}

bool TaskItem::update()
{
    bool needsUpdate = nameItem.text().isEmpty();
    if (needsUpdate)
    {
        try {
            nameItem.setText(task->getName().c_str());
        }
        catch (...)
        {
            return false;
        }
    }
    
    needsUpdate |= updateState();
    needsUpdate |= updatePorts();

    return needsUpdate;
}

bool TaskItem::updatePorts()
{
    const RTT::DataFlowInterface *dfi = task->ports();
    std::vector<std::string> portNames = dfi->getPortNames();
    bool needsUpdate = false;

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
                inputPorts.appendRow(item->getRow());
            }
            
            ports.insert(std::make_pair(portName, item));
            needsUpdate = true;
        }
        else
        {   
            item = it->second;
        }

        if (outIf)
        {
            OutputPortItem *outPortItem = static_cast<OutputPortItem *>(item);
            if (refreshPorts)
            {
                outPortItem->updateOutputPortInterface(outIf);
            }
            
            needsUpdate |= outPortItem->updataValue();
        }
    }
    
    refreshPorts = false;
    return needsUpdate;
}

bool TaskItem::updateState()
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
    
    if (statusItem.text() != stateString)
    {
        statusItem.setText(stateString);
        return true;
    }

    return false;
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

RTT::corba::TaskContextProxy* TaskItem::getTaskContext()
{
    return task;
}
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
//     std::cout << "TaskItem::updatePorts.." << std::endl;
//     std::cout << "number of properties: " << task->properties()->size() << std::endl;
    const RTT::DataFlowInterface *dfi = task->ports();
    std::vector<std::string> portNames = dfi->getPortNames();
    if (portNames.size() > 0)
    {
        std::ostringstream oss;
        std::copy(portNames.begin(), portNames.end()-1,
            std::ostream_iterator<std::string>(oss, ","));
        oss << portNames.back();
//         std::cout << "update ports " << oss.str() << std::endl;
    }
    else
    {
//         std::cout << "no port names for task " << task->getName() << std::endl;
//         std::cout << "number of ports: " << dfi->getPorts().size() << std::endl;
    }

    for(RTT::base::PortInterface *pi : dfi->getPorts())
    {
        const std::string portName(pi->getName());
//         std::cout << "update port " << portName << std::endl;
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
        }
        else
        {   
            item = it->second;
        }

        if (outIf)
        {
//             std::cout << "update port " << portName << std::endl;
            OutputPortItem *outPortItem = static_cast<OutputPortItem *>(item);
            if (refreshPorts)
            {
                outPortItem->updateOutputPortInterface(outIf);
            }
            
            outPortItem->updataValue();
        }
    }
    
    refreshPorts = false;
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

RTT::corba::TaskContextProxy* TaskItem::getTaskContext()
{
    return task;
}
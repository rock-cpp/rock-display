#include <rtt/transports/corba/TaskContextProxy.hpp>
#include "TaskItem.hpp"
#include <rtt/typelib/TypelibMarshallerBase.hpp>
#include <lib_config/TypelibConfiguration.hpp>
#include <base-logging/Logging.hpp>
#include "PropertyItem.hpp"
#include <rtt/transports/corba/RemotePorts.hpp>

TaskItem::TaskItem(RTT::corba::TaskContextProxy* _task, ConfigItemHandlerRepository *handlerrepo)
    : task(_task),
      nameItem(ItemType::TASK),
      statusItem(ItemType::TASK),
      refreshOutputPorts(true),
      refreshInputPorts(true),
      stateChanged(false),
      handlerrepo(handlerrepo)
{
    inputPorts.setText("InputPorts");
    outputPorts.setText("OutputPorts");
    properties.setText("Properties");
    nameItem.appendRow(&inputPorts);
    nameItem.appendRow(&outputPorts);
    nameItem.appendRow(&properties);
    nameItem.setData(this);
    statusItem.setData(this);
    statusItem.setText("");
}

TaskItem::~TaskItem()
{
}

void TaskItem::reset()
{
    for (auto port: ports)
    {
        OutputPortItem *outPort = dynamic_cast<OutputPortItem*>(port.second);
        if (outPort)
        {
            outPort->reset();
        }
    }
    
    task = nullptr;
}

bool TaskItem::hasVisualizers()
{
    for (auto &port: ports)
    {
        if (port.second->hasVisualizers())
        {
            return true;
        }
    }
    
    return false;
}

void TaskItem::synchronizeTask()
{
    /*task->synchronize();
     *
     * synchronize is protected, only gets published in concrete proxies
     *
     * we open code the fetchPorts() part here. we can do everything except give
     * port ownership to the task object, which is not a big problem for us.
     *
     * synchronize also checks up on operations, properties and attributes and
     * recurses into the providers and requesters
     *
     */
    auto parent = task->provides();
    auto dfact = task->server()->getProvider("this");
    RTT::corba::CDataFlowInterface::CPortDescriptions_var objs = dfact->getPortDescriptions();
    for (size_t i = 0; i < objs->length(); ++i)
    {
        RTT::corba::CPortDescription port = objs[i];
        if (parent->getPort(port.name.in()))
            continue; // already added.
        auto type_info = RTT::detail::TypeInfoRepository::Instance()->type(port.type_name.in());
        if (type_info)
        {
            RTT::base::PortInterface *new_port;
            if (port.type == RTT::corba::CInput)
                new_port = new RTT::corba::RemoteInputPort(type_info, dfact, port.name.in(), RTT::corba::TaskContextProxy::ProxyPOA());
            else
                new_port = new RTT::corba::RemoteOutputPort(type_info, dfact, port.name.in(), RTT::corba::TaskContextProxy::ProxyPOA());

            parent->addPort(*new_port);
            //the new_port is still owned by us, but we leak it for now.
        }
    }
}

void TaskItem::update(bool handleOldData)
{   
    {
        if (!task)
        {
            return;
        }
    
        if (!task->server() || task->server()->_is_nil())
        {
            LOG_WARN_S << "TaskItem::update(): disconnect of task " << task->getName();
            reset();
            return;
        }
    }

    if (nameItem.text().isEmpty())
    {
        try
        {
            nameItem.setText(task->getName().c_str());
        }
        catch (...)
        {
            return;
        }
    }
    
    // check for task state update
    stateChanged = updateState();
    
    // check for property update
    if (propertyMap.empty() || stateChanged)
    {
        synchronizeTask();
        updateProperties();
    }

    // check for port update
    bool hasVis = hasVisualizers();
    if (!hasVis && !nameItem.isExpanded())
    {
        return;
    }
    
    updatePorts(hasVis, handleOldData);
}

void TaskItem::updatePorts(bool hasVisualizers, bool handleOldData)
{
    bool refreshedOutputPorts = false;
    bool refreshedInputPorts = false;
    
    if (ports.empty() || hasVisualizers || outputPorts.isExpanded() || inputPorts.isExpanded())
    {
        const RTT::DataFlowInterface *dfi = task->ports();

        for (RTT::base::PortInterface *pi : dfi->getPorts())
        {
            const std::string portName(pi->getName());
            RTT::base::OutputPortInterface *outIf = dynamic_cast<RTT::base::OutputPortInterface *>(pi);
            RTT::base::InputPortInterface *inIf = dynamic_cast<RTT::base::InputPortInterface *>(pi);
            auto it = ports.find(portName);
            PortItem *item = nullptr;
            if (it == ports.end())
            {
                if (outIf)
                {
                    item = new OutputPortItem(outIf, handlerrepo);
                    outputPorts.appendRow(item->getRow());
                    item->getNameItem()->appendRow({new QStandardItem(QString("waiting for data..")), new QStandardItem()});
                }
                else if (inIf)
                {
                    auto inPortItem = new InputPortItem(inIf, handlerrepo);
                    item = inPortItem;
                    inputPorts.appendRow(item->getRow());
                    //the QStandardModel becomes confused when items are added later when there were none
                    //at the start.
                    item->getNameItem()->appendRow({new QStandardItem(QString("no elements(yet)")), new QStandardItem()});
                }
                else
                {
                    LOG_WARN_S << "found port item that is neither input nor output";
                    //should not happen i guess?
                    item = new PortItem(portName, handlerrepo);
                }
                
                ports.insert(std::make_pair(portName, item));
            }
            else
            {   
                item = it->second;
            }

            if (!item)
            {
                continue;
            }

            if ((item->hasVisualizers() || outputPorts.isExpanded()) && outIf)
            {
                OutputPortItem *outPortItem = static_cast<OutputPortItem *>(item);

                if (refreshOutputPorts)
                {
                    outPortItem->updateOutputPortInterface(outIf);
                    refreshedOutputPorts = true;
                }

                outPortItem->updataValue(handleOldData);
            }
            if ((item->hasVisualizers() || inputPorts.isExpanded()) && inIf)
            {
                InputPortItem *inPortItem = static_cast<InputPortItem *>(item);

                if (refreshInputPorts)
                {
                    inPortItem->updateInputPortInterface(inIf);
                    refreshedInputPorts = true;
                }

                inPortItem->updataValue(handleOldData);
            }
        }
    }
    
    if (refreshedOutputPorts)
    {
        refreshOutputPorts = false;
    }
    if (refreshedInputPorts)
    {
        refreshInputPorts = false;
    }
}

void TaskItem::updateProperties()
{   
    RTT::PropertyBag *taskProperties = task->properties();
    
    for (std::size_t i=0; i<taskProperties->size(); i++)
    {
        RTT::base::PropertyBase *property = taskProperties->getItem(i);

        auto it = propertyMap.find(property->getName());
        if (it == propertyMap.end())
        {
            PropertyItem *item = new PropertyItem(property, handlerrepo);

            propertyMap[property->getName()] = item;
            item->updataValue();
            
            properties.appendRow(item->getRow());
        }
        else
        {
            PropertyItem *item = it->second;
            item->updateProperty(property);
            item->updataValue();
        }
    }
}

bool TaskItem::updateState()
{
    std::string stateString = "";
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
    
    if (statusItem.text().toStdString() != stateString)
    {
        statusItem.setText(stateString.c_str());
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

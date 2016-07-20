#include <rtt/base/OutputPortInterface.hpp>
#include <orocos_cpp_base/OrocosHelpers.hpp>
#include <orocos_cpp_base/ProxyPort.hpp>
#include <boost/lexical_cast.hpp>
#include "PortItem.hpp"

std::string getFreePortName(RTT::TaskContext* clientTask, const RTT::base::PortInterface* portIf)
{
    int cnt = 0;
    while(true)
    {
        std::string localName = portIf->getName() + boost::lexical_cast<std::string>(cnt);
        if(clientTask->getPort(localName))
        {
            cnt++;
        }
        else
        {
            return localName;
        }
    }
}

PortItem::PortItem(const std::string& name) : nameItem(ItemType::PORT), valueItem(ItemType::PORT)
{
    nameItem.setText(name.c_str());
    valueItem.setText("Dummy");
}

QList<QStandardItem* > PortItem::getRow()
{
    return {&nameItem, &valueItem};
}

OutputPortItem::OutputPortItem(RTT::base::OutputPortInterface* port) : PortItem(port->getName())
{
    reader = dynamic_cast<RTT::base::InputPortInterface *>(port->antiClone());
    if(!reader)
        throw std::runtime_error("Error, could not get reader for port " + port->getName());
    RTT::TaskContext *clientTask = OrocosHelpers::getClientTask();
    reader->setName(getFreePortName(clientTask, port));
    clientTask->addPort(*reader);
    RTT::ConnPolicy policy(RTT::ConnPolicy::data());
    policy.pull = true;
    if(!reader->connectTo(port, policy))
        throw std::runtime_error("OutputPortItem: Error could not connect reader to port " + port->getName() + " of task " + port->getInterface()->getOwner()->getName());

}

bool OutputPortItem::updataValue()
{
}

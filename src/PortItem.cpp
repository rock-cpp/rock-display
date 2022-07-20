/* this include is first because it uses "emit" and "signals" which are
 * #defined by Qt */
#include <orocos_cpp_base/ProxyPort.hpp>
#include "PortItem.hpp"

#include <rtt/base/OutputPortInterface.hpp>
#include <orocos_cpp_base/OrocosHelpers.hpp>
#include <rtt/typelib/TypelibMarshallerBase.hpp>
#include <boost/lexical_cast.hpp>
#include "ConfigItem.hpp"
#include <lib_config/TypelibConfiguration.hpp>
#include <base-logging/Logging.hpp>
#include <QMetaType>
#include "Types.hpp"

class PortHandle
{
public:
    std::string name;
    orogen_transports::TypelibMarshallerBase *transport;
    RTT::base::DataSourceBase::shared_ptr sample;
    orogen_transports::TypelibMarshallerBase::Handle *transportHandle;
    RTT::base::PortInterface *port;
    const Typelib::Type *type;
};

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

PortItem::PortItem(const std::string& name, ConfigItemHandlerRepository *handlerrepo)
    : VisualizerAdapter(), nameItem(new TypedItem(ItemType::INPUTPORT)), valueItem(new TypedItem(ItemType::INPUTPORT)), handlerrepo(handlerrepo)
{
    nameItem->setText(name.c_str());

    nameItem->setData(this);
    valueItem->setData(this);
}

PortItem::~PortItem()
{
    delete nameItem;
    delete valueItem;
}

QList<QStandardItem* > PortItem::getRow()
{
    return {nameItem, valueItem};
}

OutputPortItem::OutputPortItem(RTT::base::OutputPortInterface* port, ConfigItemHandlerRepository *handlerrepo) : PortItem(port->getName(), handlerrepo) , handle(nullptr), needUiUpdate(false)
{
    nameItem->setType(ItemType::OUTPUTPORT);
    valueItem->setType(ItemType::OUTPUTPORT);
    nameItem->setData(this);
    valueItem->setData(this);
    typeInfo = port->getTypeInfo()->getTypeName();
    valueItem->setText(typeInfo.c_str());
}

OutputPortItem::~OutputPortItem()
{
    reset();
}

RTT::base::PortInterface* OutputPortItem::getPort()
{
    if (handle)
    {
        return this->handle->port;
    }

    return nullptr;
}

void OutputPortItem::reset()
{
    if (handle)
    {
        delete handle;
    }
    
    if (reader && reader->connected())
    {
        reader->disconnect();
    }
    
    handle = nullptr;
    reader = nullptr;
}

void OutputPortItem::updateOutputPortInterface(RTT::base::OutputPortInterface* port)
{
    reader = dynamic_cast<RTT::base::InputPortInterface *>(port->antiClone());
    if(!reader)
    {
        throw std::runtime_error("Error, could not get reader for port " + port->getName());
    }
    RTT::TaskContext *clientTask = OrocosHelpers::getClientTask();
    reader->setName(getFreePortName(clientTask, port));
    clientTask->addPort(*reader);

    handle = new PortHandle();
    handle->port = port;
    RTT::types::TypeInfo const *type = port->getTypeInfo();
    handle->transport = dynamic_cast<orogen_transports::TypelibMarshallerBase *>(type->getProtocol(orogen_transports::TYPELIB_MARSHALLER_ID));
    if (! handle->transport)
    {
        LOG_ERROR_S << "cannot report ports of type " << type->getTypeName() << " as no typekit generated by orogen defines it";
        delete handle;
        handle = nullptr;
        return;
    }

    handle->transportHandle = handle->transport->createSample();
    handle->sample = handle->transport->getDataSource(handle->transportHandle);

    handle->type = handle->transport->getRegistry().get(handle->transport->getMarshallingType());
}

void OutputPortItem::updataValue(bool updateUI, bool handleOldData)
{    
    if (!handle || !reader)
    {
        return;
    }
    
    if (!reader->connected())
    {
        RTT::ConnPolicy policy(RTT::ConnPolicy::data());
        policy.pull = true;
        if(!reader->connectTo(handle->port, policy))
        {
            LOG_ERROR_S << "OutputPortItem: Error could not connect reader to port " + handle->port->getName() + " of task " + handle->port->getInterface()->getOwner()->getName();
            return;
        }
    }
    
    if (reader->read(handle->sample) == RTT::NewData || handleOldData)
    {
        needUiUpdate = true;
    }
    else if (!updateUI || !needUiUpdate)
    {
        return;
    }

    handle->transport->refreshTypelibSample(handle->transportHandle);
    
    emit visualizerUpdate(handle->sample.get()->getRawConstPointer(), handle->sample);

    Typelib::Value val(handle->transport->getTypelibSample(handle->transportHandle), *(handle->type));
    
    if (!item && updateUI)
    {   
        while (nameItem->rowCount() > 0)
        {
            nameItem->takeRow(0);
        }
        
        item = getItem(val, handlerrepo, this->nameItem, this->valueItem);
        item->update(val, handle->sample, updateUI, true);
    }
    else if (item)
    {
        item->update(val, handle->sample, updateUI);
    }

    if(updateUI)
    {
        needUiUpdate = false;
    }
}

const std::string& OutputPortItem::getType()
{
    return typeInfo;
}

Typelib::Value OutputPortItem::getValueHandle()
{
    return Typelib::Value(handle->transport->getTypelibSample(handle->transportHandle), *(handle->type));
}

RTT::base::DataSourceBase::shared_ptr OutputPortItem::getBaseSample()
{
    return handle->sample;
}

InputPortItem::InputPortItem(RTT::base::InputPortInterface* port, ConfigItemHandlerRepository *handlerrepo) : PortItem(port->getName(), handlerrepo) , handle(nullptr), writer(nullptr)
{
    nameItem->setType(ItemType::INPUTPORT);
    valueItem->setType(ItemType::INPUTPORT);
    nameItem->setData(this);
    valueItem->setData(this);
    typeInfo = port->getTypeInfo()->getTypeName();
    valueItem->setText(typeInfo.c_str());
    updateInputPortInterface(port);
}

InputPortItem::~InputPortItem()
{
    reset();
}

RTT::base::PortInterface* InputPortItem::getPort()
{
    if (handle)
    {
        return this->handle->port;
    }

    return nullptr;
}

void InputPortItem::reset()
{
    if (handle)
    {
        delete handle;
    }

    if (writer && writer->connected())
    {
        writer->disconnect();
    }

    handle = nullptr;
    writer = nullptr;

    if (oldDataBuffer.size() != 0)
    {
        Typelib::destroy(oldData);
        oldDataBuffer.resize(0);
    }
}

void InputPortItem::updateInputPortInterface(RTT::base::InputPortInterface* port)
{
    writer = dynamic_cast<RTT::base::OutputPortInterface *>(port->antiClone());
    if(!writer)
    {
        throw std::runtime_error("Error, could not get writer for port " + port->getName());
    }
    RTT::TaskContext *clientTask = OrocosHelpers::getClientTask();
    writer->setName(getFreePortName(clientTask, port));
    clientTask->addPort(*writer);

    handle = new PortHandle();
    handle->port = port;
    RTT::types::TypeInfo const *type = port->getTypeInfo();
    handle->transport = dynamic_cast<orogen_transports::TypelibMarshallerBase *>(type->getProtocol(orogen_transports::TYPELIB_MARSHALLER_ID));
    if (! handle->transport)
    {
        LOG_ERROR_S << "cannot edit ports of type " << type->getTypeName() << " as no typekit generated by orogen defines it";
        delete handle;
        handle = nullptr;
        return;
    }

    handle->transportHandle = handle->transport->createSample();
    handle->sample = handle->transport->getDataSource(handle->transportHandle);

    handle->type = handle->transport->getRegistry().get(handle->transport->getMarshallingType());
}

void InputPortItem::updataValue(bool updateUI, bool handleOldData)
{
    if (!handle)
    {
        return;
    }

    if (oldDataBuffer.size() != 0 && !handleOldData)
    {
        return;
    }

    handle->transport->refreshTypelibSample(handle->transportHandle);

    emit visualizerUpdate(handle->sample.get()->getRawConstPointer(), handle->sample);

    currentData = Typelib::Value(handle->transport->getTypelibSample(handle->transportHandle), *(handle->type));

    if (oldDataBuffer.size() == 0)
    {
        Typelib::Type const &type = currentData.getType();
        oldDataBuffer.resize(type.getSize());
        Typelib::init(oldDataBuffer.data(), Typelib::layout_of(type));
        oldData = Typelib::Value(oldDataBuffer.data(), type);
        Typelib::copy(oldData, currentData);
    }

    if (!item && updateUI)
    {
        while (nameItem->rowCount() > 0)
        {
            nameItem->takeRow(0);
        }

        item = getEditableItem(currentData, handlerrepo, this->nameItem, this->valueItem);
        item->update(currentData, handle->sample, updateUI, true);
    }

    item->update(currentData, handle->sample, updateUI);
}

const std::string& InputPortItem::getType()
{
    return typeInfo;
}

Typelib::Value &InputPortItem::getCurrentData()
{
    return currentData;
}

Typelib::Value &InputPortItem::getOldData()
{
    return oldData;
}

void InputPortItem::sendCurrentData()
{
    if (!writer)
        return;

    if (!writer->connected())
    {
        RTT::ConnPolicy policy(RTT::ConnPolicy::data());
        if(!writer->connectTo(handle->port, policy))
        {
            LOG_ERROR_S << "InputPortItem: Error could not connect writer to port " + handle->port->getName() + " of task " + handle->port->getInterface()->getOwner()->getName();
            return;
        }
    }

    writer->write(handle->sample);

    Typelib::copy(oldData, currentData);
}

void InputPortItem::restoreOldData()
{
    Typelib::copy(currentData, oldData);
}

bool InputPortItem::compareAndMarkData()
{
    return item->compareAndMark(currentData, oldData);
}

Typelib::Value InputPortItem::getValueHandle()
{
    return currentData;
}

RTT::base::DataSourceBase::shared_ptr InputPortItem::getBaseSample()
{
    return handle->sample;
}

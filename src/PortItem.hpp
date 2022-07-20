#pragma once

#include "ConfigItem.hpp"
#include "VisualizerAdapter.hpp"

QT_BEGIN_NAMESPACE
class QStandardItem;
QT_END_NAMESPACE

namespace RTT
{
    namespace base
    {
        class OutputPortInterface;
        class InputPortInterface;
        class PortInterface;
    }
}

class TypedItem;

class PortItem : public VisualizerAdapter
{
protected:
    std::shared_ptr<ItemBase> item;
    TypedItem *nameItem;
    TypedItem *valueItem;
    std::string typeInfo;
    ConfigItemHandlerRepository *handlerrepo;

public:
    PortItem(const std::string &name, ConfigItemHandlerRepository *handlerrepo);
    virtual ~PortItem();
    QList<QStandardItem *> getRow();
    
    std::shared_ptr<ItemBase> getItemBase()
    {
        return item;
    }
    
    TypedItem *getNameItem()
    {
        return nameItem;
    }
    
    TypedItem *getValueItem()
    {
        return valueItem;
    }
    
    virtual bool hasVisualizers()
    {
        if (VisualizerAdapter::hasVisualizers() || (item && item->hasVisualizers()))
        {
            return true;
        }
        
        return false;
    }
    virtual Typelib::Value getValueHandle() { return Typelib::Value(); }
    virtual RTT::base::DataSourceBase::shared_ptr getBaseSample() { return nullptr; }
};

class PortHandle;

class OutputPortItem : public PortItem
{
    PortHandle *handle;
    RTT::base::InputPortInterface *reader;
    bool needUiUpdate;
    
public:
    OutputPortItem(RTT::base::OutputPortInterface* port, ConfigItemHandlerRepository *handlerrepo);
    virtual ~OutputPortItem();
    void updataValue(bool updateUI = true, bool handleOldData = false);
    
    void updateOutputPortInterface(RTT::base::OutputPortInterface* port);
    const std::string &getType();
    RTT::base::PortInterface* getPort();
    
    void reset();
    virtual Typelib::Value getValueHandle() override;
    virtual RTT::base::DataSourceBase::shared_ptr getBaseSample() override;
};

class InputPortItem : public PortItem
{
    PortHandle *handle;
    Typelib::Value currentData;
    Typelib::Value oldData;
    std::vector<uint8_t> oldDataBuffer;
    RTT::base::OutputPortInterface *writer;

public:
    InputPortItem(RTT::base::InputPortInterface* port, ConfigItemHandlerRepository *handlerrepo);
    virtual ~InputPortItem();
    void updataValue(bool updateUI = true, bool handleOldData = false);
    bool compareAndMarkData();

    void updateInputPortInterface(RTT::base::InputPortInterface* port);
    const std::string &getType();
    RTT::base::PortInterface* getPort();
    Typelib::Value &getCurrentData();
    Typelib::Value &getOldData();

    void reset();
    void sendCurrentData();
    void restoreOldData();

    virtual Typelib::Value getValueHandle() override;
    virtual RTT::base::DataSourceBase::shared_ptr getBaseSample() override;
};

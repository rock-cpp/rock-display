
#pragma once

#include <QObject>
#include <rtt/typelib/TypelibMarshallerBase.hpp>

QT_BEGIN_NAMESPACE
class QStandardItem;
QT_END_NAMESPACE

class ConfigItemHandlerRepository;
class TypedItem;
class ItemBase;

class PropertyItem : public QObject
{
    std::shared_ptr<ItemBase> item;
    TypedItem *nameItem;
    TypedItem *valueItem;
    std::string typeInfo;
    const Typelib::Type *type;
    ConfigItemHandlerRepository *handlerrepo;
    Typelib::Value currentData;

    orogen_transports::TypelibMarshallerBase *transport;
    RTT::base::DataSourceBase::shared_ptr sample;
    orogen_transports::TypelibMarshallerBase::Handle *transportHandle;
    RTT::base::DataSourceBase::shared_ptr propertyDataSource;

public:
    PropertyItem(RTT::base::PropertyBase *property, ConfigItemHandlerRepository *handlerrepo);
    virtual ~PropertyItem();
    void updataValue(bool updateUI = true);
    QList<QStandardItem *> getRow();
    std::shared_ptr<ItemBase> getItemBase()
    {
        return item;
    }

    void updateProperty(RTT::base::PropertyBase* property);
    const std::string &getType();
    Typelib::Value &getCurrentData();

    void reset();
    void setCurrentData();

    virtual Typelib::Value getValueHandle();
    virtual RTT::base::DataSourceBase::shared_ptr getBaseSample();
};


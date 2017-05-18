#pragma once

#include <lib_config/Configuration.hpp>
#include "TypedItem.hpp"
#include <typelib/value.hh>
#include <QTextCodec>
#include <rtt/typelib/TypelibMarshallerBase.hpp>
#include <orocos_cpp/PluginHelper.hpp>

class PortHandle;
class OutputPortItem;
class VizHandle;
class PortItem;

namespace RTT
{
    namespace types
    {
        class TypeInfo;
    }
    
    namespace base
    {
        class OutputPortInterface;
        class InputPortInterface;
        class PortInterface;
    }
}

class VisualizerAdapter
{
protected:
    std::map<std::string, VizHandle> visualizers;
    
public:
    VisualizerAdapter()
    {
    };
    
    virtual ~VisualizerAdapter()
    {
    }
    
    void addPlugin(const std::string &name, VizHandle handle);
    bool hasVisualizer(const std::string &name);
    QObject *getVisualizer(const std::string &name);
    bool removeVisualizer(QObject *plugin);
    virtual bool hasVisualizers()
    {
//         std::cout << "VisualizerAdapter::hasVisualizers().." << std::endl;
        return visualizers.empty();
    }
};

class ItemBase : public VisualizerAdapter
{
protected:
    TypedItem *name;
    TypedItem *value;
    QTextCodec *codec;
    
    QTextCodec::ConverterState state;
    std::vector<std::shared_ptr<ItemBase> > children;
    
public:
    ItemBase();
    ItemBase(TypedItem *name, TypedItem *value);
    virtual ~ItemBase();
    
    virtual bool hasVisualizers();
    
    virtual bool update(Typelib::Value& valueIn, bool updateUI = true, bool forceUpdate = false) = 0;
    void setName(const QString &newName)
    {
        name->setText(newName);
    }
    
    QList<QStandardItem *> getRow()
    {
        return {name, value};
    }
    
    void setType(int newType)
    {
        this->name->setType(newType);
        this->value->setType(newType);
    }
    
    std::vector<std::shared_ptr<ItemBase> > getChildren()
    {
        return this->children;
    }
    
    TypedItem *getName()
    {
        return this->name;
    }
};

std::shared_ptr<ItemBase> getItem(Typelib::Value& value, TypedItem *nameItem = nullptr, TypedItem *valueItem = nullptr);

class Array : public ItemBase
{   
    const int maxArrayElemsDisplayed = 500;
    
public:
    Array(Typelib::Value& valueIn, TypedItem *name = nullptr, TypedItem *value = nullptr);
    virtual ~Array();
    
    virtual bool update(Typelib::Value& valueIn, bool updateUI = false, bool forceUpdate = false);
};

class Simple : public ItemBase
{
public:
    Simple(Typelib::Value& valueIn, TypedItem *name = nullptr, TypedItem *value = nullptr);    
    virtual ~Simple();
    
    virtual bool update(Typelib::Value& valueIn, bool updateUI = false, bool forceUpdate = false);
};

class Complex : public ItemBase
{
    const int maxVectorElemsDisplayed = 500;
    orogen_transports::TypelibMarshallerBase *transport;
    orogen_transports::TypelibMarshallerBase::Handle *transportHandle;
    RTT::base::DataSourceBase::shared_ptr sample;
    
public:
    Complex(Typelib::Value& valueIn, TypedItem *name = nullptr, TypedItem *value = nullptr);
    virtual ~Complex();
    
    virtual bool update(Typelib::Value& valueIn, bool updateUI = false, bool forceUpdate = false);
};
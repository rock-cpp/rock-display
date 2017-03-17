#pragma once

#include <lib_config/Configuration.hpp>
#include "TypedItem.hpp"
#include <typelib/value.hh>
#include <QTextCodec>

class PortHandle;
class OutputPortItem;
class VizHandle;

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
    ~VisualizerAdapter()
    {
        
    }
    void addPlugin(const std::string &name, VizHandle handle);
    bool hasVisualizer(const std::string &name);
    QObject *getVisualizer(const std::string &name);
    bool removeVisualizer(QObject *plugin);
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
    virtual ~ItemBase();
    
    virtual bool hasActiveVisualizers();
    
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

std::shared_ptr<ItemBase> getItem(Typelib::Value& value);

class Array : public ItemBase
{   
    const std::size_t maxArrayElemsDisplayed = 500;
    
public:
    Array(Typelib::Value& valueIn);
    virtual ~Array();
    
    virtual bool update(Typelib::Value& valueIn, bool updateUI = false, bool forceUpdate = false);
    virtual bool hasActiveVisualizers();
};

class Simple : public ItemBase
{
public:
    Simple(Typelib::Value& valueIn);    
    virtual ~Simple();
    
    virtual bool update(Typelib::Value& valueIn, bool updateUI = false, bool forceUpdate = false);
};

class Complex : public ItemBase
{
    const std::size_t maxVectorElemsDisplayed = 500;
    
public:
    Complex(Typelib::Value& valueIn);
    virtual ~Complex();
    
    virtual bool update(Typelib::Value& valueIn, bool updateUI = false, bool forceUpdate = false);
    virtual bool hasActiveVisualizers();
};
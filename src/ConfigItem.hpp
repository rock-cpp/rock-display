#pragma once

#include <lib_config/Configuration.hpp>
#include "TypedItem.hpp"
#include <typelib/value.hh>

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

class ItemBase
{   
public:
    TypedItem *name;
    TypedItem *value;
    
    std::vector<VizHandle> activeVizualizer;
    
    void addPlugin(VizHandle &handle);

    ItemBase();
    
    virtual void update(Typelib::Value& valueIn) = 0;
    virtual ~ItemBase();
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
};

std::shared_ptr<ItemBase> getItem(Typelib::Value& value);

class Array : public ItemBase
{
    std::vector<std::shared_ptr<ItemBase> > childs;
    
public:
    Array(Typelib::Value& valueIn);
    virtual ~Array();
    virtual void update(Typelib::Value& valueIn);
};

class Simple : public ItemBase
{
public:
    Simple(Typelib::Value& valueIn);    
    virtual ~Simple();
    virtual void update(Typelib::Value& valueIn);
};

class Complex : public ItemBase
{
    std::vector<std::shared_ptr<ItemBase> > childs;
    
public:
    Complex(Typelib::Value& valueIn);
    virtual ~Complex();
    virtual void update(Typelib::Value& valueIn);
};
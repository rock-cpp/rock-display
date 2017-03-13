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

class ItemBase
{
protected:
    TypedItem *name;
    TypedItem *value;
    QTextCodec *codec;
    bool expanded;
    
    QTextCodec::ConverterState state;
    std::vector<std::shared_ptr<ItemBase> > children;
    
public:
    ItemBase();
    virtual ~ItemBase();
    
    virtual bool hasActiveVisualizers();
    
    std::map<std::string, VizHandle> activeVizualizer;
    
    void addPlugin(std::pair<std::string, VizHandle> handle);
    bool hasVizualizer(const std::string &name);
    QObject *getVizualizer(const std::string &name);
    void removeVizualizer(QObject *plugin);
    
    virtual bool update(Typelib::Value& valueIn, bool updateUI = true) = 0;
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
    
    void setExpanded(bool expanded)
    {
        this->expanded = expanded;
    }
    
    bool isExpanded()
    {
        return this->expanded;
    }
    
    std::vector<std::shared_ptr<ItemBase> > getChildren()
    {
        return this->children;
    }
};

std::shared_ptr<ItemBase> getItem(Typelib::Value& value);

class Array : public ItemBase
{   
public:
    Array(Typelib::Value& valueIn);
    virtual ~Array();
    
    virtual bool update(Typelib::Value& valueIn, bool updateUI = false);
    virtual bool hasActiveVisualizers();
};

class Simple : public ItemBase
{
public:
    Simple(Typelib::Value& valueIn);    
    virtual ~Simple();
    
    virtual bool update(Typelib::Value& valueIn, bool updateUI = false);
};

class Complex : public ItemBase
{
    const std::size_t maxVectorElemsShown = 500;
    
public:
    Complex(Typelib::Value& valueIn);
    virtual ~Complex();
    
    virtual bool update(Typelib::Value& valueIn, bool updateUI = false);
    virtual bool hasActiveVisualizers();
};
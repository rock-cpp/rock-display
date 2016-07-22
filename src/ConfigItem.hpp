#pragma once

#include <lib_config/Configuration.hpp>
#include "TypedItem.hpp"

class ItemBase
{
protected:
    TypedItem *name;
    TypedItem *value;

public:
    ItemBase();
    
    virtual void update(const std::shared_ptr< libConfig::ConfigValue >& value) = 0;
    virtual ~ItemBase();
    void setName(const QString &newName)
    {
        name->setText(newName);
    }
    
    void setType(int newType, void *data);
    
    QList<QStandardItem *> getRow()
    {
        return {name, value};
    }
};

std::shared_ptr<ItemBase> getItem(const std::shared_ptr< libConfig::ConfigValue >& value);

class Array : public ItemBase
{
    std::vector<std::shared_ptr<ItemBase> > childs;
public:
    Array(const std::shared_ptr< libConfig::ConfigValue >& valueIn);
    virtual ~Array();
    virtual void update(const std::shared_ptr< libConfig::ConfigValue >& valueIn);
};

class Simple : public ItemBase
{
public:
    Simple(const std::shared_ptr< libConfig::ConfigValue >& valueIn);    
    virtual ~Simple();
    virtual void update(const std::shared_ptr< libConfig::ConfigValue >& valueIn);
};

class Complex : public ItemBase
{
    std::vector<std::shared_ptr<ItemBase> > childs;
public:
    Complex(const std::shared_ptr< libConfig::ConfigValue >& valueIn);
    virtual ~Complex();
    virtual void update(const std::shared_ptr< libConfig::ConfigValue >& value);
};




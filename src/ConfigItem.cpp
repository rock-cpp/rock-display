#include "ConfigItem.hpp"


class ItemBase
{
protected:
    QStandardItem name;
    QStandardItem value;

public:
    virtual void update(const std::shared_ptr< libConfig::ConfigValue >& value) = 0;

    void setName(const QString &newName)
    {
        name.setText(newName);
    }
    
    QList<QStandardItem *> getRow()
    {
        return {&name, &value};
    }
};

std::shared_ptr<ItemBase> getItem(const std::shared_ptr< libConfig::ConfigValue >& value);

class Array : public ItemBase
{
    std::vector<std::shared_ptr<ItemBase>> childs;
public:
    Array(const std::shared_ptr< libConfig::ConfigValue >& valueIn)
    {
        update(valueIn);
    }
    
    virtual void update(const std::shared_ptr< libConfig::ConfigValue >& valueIn)
    {
        libConfig::ArrayConfigValue *aVal = static_cast<libConfig::ArrayConfigValue *>(valueIn.get());
        auto values(aVal->getValues());

        if(values.size() < childs.size())
        {
            name.removeRows(childs.size(), values.size());
            childs.resize(values.size());
        }
        
        for(size_t i = 0 ; i < childs.size(); i++)
        {
            childs[i]->update(values[i]);
        }
        
        
        for(size_t i = childs.size(); i < values.size(); i++)
        {
            std::shared_ptr<ItemBase> newVal = getItem(values[i]);
            newVal->setName(QString::number(i));
            childs.push_back(newVal);
            name.appendRow(newVal->getRow());
        }
    }
};

class Simple : public ItemBase
{
public:
    Simple(const std::shared_ptr< libConfig::ConfigValue >& valueIn)
    {
        update(valueIn);
    };
    
    void update(const std::shared_ptr< libConfig::ConfigValue >& valueIn)
    {
        libConfig::SimpleConfigValue *sVal = static_cast<libConfig::SimpleConfigValue *>(valueIn.get());
        name.setText(sVal->getName().c_str());
        value.setText(sVal->getValue().c_str());        
    }
};

class Complex : public ItemBase
{
    std::vector<std::shared_ptr<ItemBase>> childs;

public:
    Complex(const std::shared_ptr< libConfig::ConfigValue >& valueIn)
    {
        update(valueIn);
    }
    
    virtual void update(const std::shared_ptr< libConfig::ConfigValue >& valueIn)
    {
        libConfig::ComplexConfigValue *cVal = static_cast<libConfig::ComplexConfigValue *>(valueIn.get());

        size_t i = 0;
        for(auto val : cVal->getValues())
        {
            if(childs.size() <= i)
            {
                std::shared_ptr<ItemBase> newVal = getItem(val.second);
                newVal->setName(val.first.c_str());
                childs.push_back(newVal);
                name.appendRow(newVal->getRow());
                i++;
                continue;
            }
            
            childs[i]->update(val.second);
            i++;
        }
    }
};

std::shared_ptr<ItemBase> getItem(const std::shared_ptr< libConfig::ConfigValue >& value)
{
    switch(value->getType())
    {
        case libConfig::ConfigValue::ARRAY:
            return std::shared_ptr<ItemBase>(new Array(value));
            break;
        case libConfig::ConfigValue::COMPLEX:
            return std::shared_ptr<ItemBase>(new Complex(value));
            break;
        case libConfig::ConfigValue::SIMPLE:
            return std::shared_ptr<ItemBase>(new Simple(value));
            break;
    }
}


ConfigItem::ConfigItem()
{

}

QList< QStandardItem* > ConfigItem::getValue(const std::shared_ptr< libConfig::ConfigValue >& value)
{
    switch(value->getType())
    {
        case libConfig::ConfigValue::ARRAY:
            return getArray(value);
            break;
        case libConfig::ConfigValue::COMPLEX:
            return getComplex(value);
            break;
        case libConfig::ConfigValue::SIMPLE:
            return getSimple(value);
            break;
    }
    
    std::cout << "ERROR" << std::endl;
    
    return {};
}

QList< QStandardItem* > ConfigItem::getComplex(const std::shared_ptr< libConfig::ConfigValue >& value)
{
    libConfig::ComplexConfigValue *cVal = static_cast<libConfig::ComplexConfigValue *>(value.get());
    QStandardItem *topName = new QStandardItem(cVal->getName().c_str());
    QStandardItem *typeName = new QStandardItem(cVal->getCxxTypeName().c_str());
    
    for(auto &v : cVal->getValues())
    {
        auto list = getValue(v.second);
        list.front()->setText(v.first.c_str());
//         list.insert(0, new QStandardItem(v.second->getName().c_str()));
        topName->appendRow( list );
    }
    
    return {topName, typeName};
}

QList< QStandardItem* > ConfigItem::getSimple(const std::shared_ptr< libConfig::ConfigValue >& value)
{
    libConfig::SimpleConfigValue *sVal = static_cast<libConfig::SimpleConfigValue *>(value.get());
    
    return {new QStandardItem(sVal->getName().c_str()), new QStandardItem(sVal->getValue().c_str())};
}

QList< QStandardItem* > ConfigItem::getArray(const std::shared_ptr< libConfig::ConfigValue >& value)
{
    libConfig::ArrayConfigValue *aVal = static_cast<libConfig::ArrayConfigValue *>(value.get());
    QStandardItem *topName = new QStandardItem(aVal->getName().c_str());
    QStandardItem *typeName = new QStandardItem(aVal->getCxxTypeName().c_str());
    
    size_t cnt = 0;
    for(auto &v : aVal->getValues())
    {
        auto list = getValue(v);
        list.front()->setText(QString::number(cnt));
        topName->appendRow(list);
        cnt++;
    }
    
    return {topName, typeName};
}

void ConfigItem::update(const std::shared_ptr< libConfig::ConfigValue >& value, QStandardItem* parent)
{
    parent->removeColumns(0, parent->columnCount());
    parent->removeRows(0, parent->rowCount());

    parent->appendRow(getValue(value));
}

// QList< QStandardItem* >  ConfigItem::update(std::shared_ptr< libConfig::ConfigValue >& value)
// {
//     removeColumns(0, columnCount());
//     removeRows(0, rowCount());
//     
//     
// }

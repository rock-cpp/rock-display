#include "ConfigItem.hpp"

ItemBase::ItemBase() : name(new TypedItem()), value(new TypedItem())
{

}

ItemBase::~ItemBase()
{

}

void ItemBase::setType(int newType, void* data)
{
    name->setType(newType);
    value->setType(newType);
}

Array::Array(const std::shared_ptr< libConfig::ConfigValue >& valueIn)
{
    value->setText(valueIn->getCxxTypeName().c_str());
    update(valueIn);
}

Array::~Array()
{

}

void Array::update(const std::shared_ptr< libConfig::ConfigValue >& valueIn)
{
    libConfig::ArrayConfigValue *aVal = static_cast<libConfig::ArrayConfigValue *>(valueIn.get());
    auto values(aVal->getValues());

    if(values.size() < childs.size())
    {
        name->removeRows(childs.size(), values.size());
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
        name->appendRow(newVal->getRow());
    }
}

Simple::Simple(const std::shared_ptr< libConfig::ConfigValue >& valueIn)
{
    name->setText(valueIn->getName().c_str());
    update(valueIn);
}

Simple::~Simple()
{

}

void Simple::update(const std::shared_ptr< libConfig::ConfigValue >& valueIn)
{
    libConfig::SimpleConfigValue *sVal = static_cast<libConfig::SimpleConfigValue *>(valueIn.get());
    value->setText(sVal->getValue().c_str());
}

Complex::Complex(const std::shared_ptr< libConfig::ConfigValue >& valueIn)
{
    update(valueIn);
    value->setText(valueIn->getCxxTypeName().c_str());
}

Complex::~Complex()
{

}

void Complex::update(const std::shared_ptr< libConfig::ConfigValue >& valueIn)
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
            name->appendRow(newVal->getRow());
            i++;
            continue;
        }
        
        childs[i]->update(val.second);
        i++;
    }
}

std::shared_ptr< ItemBase > getItem(const std::shared_ptr< libConfig::ConfigValue >& value)
{
    switch(value->getType())
    {
        case libConfig::ConfigValue::ARRAY:
            return std::shared_ptr<ItemBase>( new Array(value));
            break;
        case libConfig::ConfigValue::SIMPLE:
            return std::shared_ptr<ItemBase>( new Simple(value));
            break;
        case libConfig::ConfigValue::COMPLEX:
            return std::shared_ptr<ItemBase>( new Complex(value));
            break;
    }
    
    throw std::runtime_error("Internal Error");
}


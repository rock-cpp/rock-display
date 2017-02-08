#include "ConfigItem.hpp"
#include <rtt/plugin/PluginLoader.hpp>
#include <rtt/types/TypeInfoRepository.hpp>
#include <orocos_cpp_base/OrocosHelpers.hpp>
#include <boost/lexical_cast.hpp>
#include <rtt/typelib/TypelibMarshallerBase.hpp>
#include <rtt/base/InputPortInterface.hpp>
#include <rtt/base/OutputPortInterface.hpp>
#include <rtt/types/TypeInfo.hpp>
#include <lib_config/TypelibConfiguration.hpp>
#include <base/Trajectory.hpp>
#include "Types.hpp"


ItemBase::ItemBase() : name(new TypedItem()), value(new TypedItem())
{
    name->setType(ItemType::CONFIGITEM);
    value->setType(ItemType::CONFIGITEM);
}

ItemBase::~ItemBase()
{

}

Array::Array(Typelib::Value& valueIn)
    : ItemBase()
{
    const Typelib::Array &array = static_cast<const Typelib::Array &>(valueIn.getType());
    value->setText(array.getName().c_str());
    update(valueIn);
}

Array::~Array()
{

}

void Array::update(Typelib::Value& valueIn)
{
    const Typelib::Array &array = static_cast<const Typelib::Array &>(valueIn.getType());
    
    void *data = valueIn.getData();
    
    const Typelib::Type &indirect(array.getIndirection());
    
    name->setData(this);
    value->setData(this);
    
    if(array.getDimension() < childs.size())
    {
        name->removeRows(childs.size(), array.getDimension());
        childs.resize(array.getDimension());
    }
    
    for(size_t i = 0; i < childs.size(); i++)
    {
        Typelib::Value arrayV(static_cast<uint8_t *>(data) + i * indirect.getSize(), indirect);
        childs[i]->update(arrayV);
    }
    
    for(size_t i = childs.size(); i < array.getDimension(); i++)
    {
        Typelib::Value arrayV(static_cast<uint8_t *>(data) + i * indirect.getSize(), indirect);
        std::shared_ptr<ItemBase> newVal = getItem(arrayV);
        newVal->setName(QString::number(i));
        childs.push_back(newVal);
        name->appendRow(newVal->getRow());
    }
}

Simple::Simple(Typelib::Value& valueIn)
    : ItemBase()
{
    libConfig::TypelibConfiguration tc;
    std::shared_ptr<libConfig::ConfigValue> conf = tc.getFromValue(valueIn);
    name->setText(conf->getName().c_str());
    update(valueIn);
}

Simple::~Simple()
{

}

void Simple::update(Typelib::Value& valueIn)
{
    libConfig::TypelibConfiguration tc;
    std::shared_ptr<libConfig::ConfigValue> conf = tc.getFromValue(valueIn);
    
    libConfig::SimpleConfigValue *sVal = static_cast<libConfig::SimpleConfigValue *>(conf.get());
    value->setText(sVal->getValue().c_str());
    name->setData(this);
    value->setData(this);
}

Complex::Complex(Typelib::Value& valueIn)
    : ItemBase()
{
    update(valueIn);
    value->setText(valueIn.getType().getName().c_str());
}

Complex::~Complex()
{

}

void Complex::update(Typelib::Value& valueIn)
{
    const Typelib::Type &type(valueIn.getType());
    name->setData(this);
    value->setData(this);
    
    for (VizHandle vizHandle : activeVizualizer)
    {
        QGenericArgument data("void *", valueIn.getData());
        vizHandle.method.invoke(vizHandle.plugin, data);
    }
    
    
    if (type.getCategory() == Typelib::Type::Compound)
    {
        const Typelib::Compound &comp = static_cast<const Typelib::Compound &>(valueIn.getType());
     
        uint8_t *data = static_cast<uint8_t *>(valueIn.getData());
        
        size_t i = 0;
        for(const Typelib::Field &field: comp.getFields())
        {
            Typelib::Value fieldV(data + field.getOffset(), field.getType());
            
            if(childs.size() <= i)
            {
                std::shared_ptr<ItemBase> newVal = getItem(fieldV);
                newVal->setName(field.getName().c_str());
                childs.push_back(newVal);
                name->appendRow(newVal->getRow());
                i++;
                continue;
            }

            childs[i]->update(fieldV);           
            i++;
        }
    }
    else
    {
        const Typelib::Container &cont = static_cast<const Typelib::Container &>(valueIn.getType());
        const size_t size = cont.getElementCount(valueIn.getData());
        
        if(cont.kind() == "/std/string")
        {
            const std::string *content = static_cast<const std::string *>(valueIn.getData());
            value->setText(content->c_str());
            return;
        }
        
        //std::vector
        if (size < childs.size())
        {
            name->removeRows(childs.size(), size);
            childs.resize(size);
        }
        
        for(size_t i = 0; i < childs.size(); i++)
        {
            Typelib::Value elem = cont.getElement(valueIn.getData(), i);
            childs[i]->update(elem);
        }
        
        for(size_t i = childs.size(); i < size; i++)
        {
            Typelib::Value elem = cont.getElement(valueIn.getData(), i);
            std::shared_ptr<ItemBase> newVal = getItem(elem);
            newVal->setName(QString::number(i));
            childs.push_back(newVal);
            name->appendRow(newVal->getRow());
        }
    }
}

std::shared_ptr< ItemBase > getItem(Typelib::Value& value)
{   
    const Typelib::Type &type(value.getType());
    
    switch(type.getCategory())
    {
        case Typelib::Type::Array:
            return std::shared_ptr<ItemBase>(new Array(value));
            break;
        case Typelib::Type::Enum:
        case Typelib::Type::Numeric:
            return std::shared_ptr<ItemBase>(new Simple(value));
            break;
        case Typelib::Type::Compound:
        case Typelib::Type::Container:
            return std::shared_ptr<ItemBase>(new Complex(value));
            break;
    }

    throw std::runtime_error("Internal Error");
}

void ItemBase::addPlugin(VizHandle& handle)
{
    activeVizualizer.push_back(handle);
}
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
    name->setData(this);
    value->setData(this);
}

ItemBase::~ItemBase()
{

}

bool ItemBase::hasActiveVisualizers()
{
    if (!activeVizualizer.empty())
    {
        return true;
    }
    
    return false;
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

bool Array::hasActiveVisualizers()
{
    if (ItemBase::hasActiveVisualizers())
    {
        return true;
    }
    
    for (auto child: childs)
    {
        if (child->hasActiveVisualizers())
        {
            return true;
        }
    }
    
    return false;
}

void Array::update(Typelib::Value& valueIn)
{
    const Typelib::Array &array = static_cast<const Typelib::Array &>(valueIn.getType());
    
    void *data = valueIn.getData();
    
    const Typelib::Type &indirect(array.getIndirection());
    
//     if (array.getDimension() < childs.size())
//     {
//         name->removeRows(array.getDimension()-1, childs.size() - array.getDimension());
//         std::cout << "Array::update(): remove rows.." << std::endl;
//         childs.resize(array.getDimension());
//     }
    
    for (size_t i = 0; i < childs.size(); i++)
    {
        Typelib::Value arrayV(static_cast<uint8_t *>(data) + i * indirect.getSize(), indirect);
        childs[i]->update(arrayV);
    }
    
    for (size_t i = childs.size(); i < array.getDimension(); i++)
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
    name->setText(valueIn.getType().getName().c_str());   
    update(valueIn);
}

Simple::~Simple()
{

}

template <class T>
std::string getValue(const Typelib::Value& value)
{
    T *val = static_cast<T *>(value.getData());
    return  boost::lexical_cast<std::string>(*val);
}

void Simple::update(Typelib::Value& valueIn)
{
    if (!value->parent() || !value->model() || value->model()->rowCount() <= 0 || value->row() < 0 || !value->index().isValid())
    {
        return;
    }
    
    const Typelib::Type &type(valueIn.getType());
    std::string valueS = value->text().toStdString();
    std::string oldValue = valueS;
    
    if (type.getCategory() == Typelib::Type::Enum)
    {
        const Typelib::Enum &enumT = static_cast<const Typelib::Enum &>(valueIn.getType());
        Typelib::Enum::integral_type *intVal = (static_cast<Typelib::Enum::integral_type *>(valueIn.getData()));
        valueS = enumT.get(*intVal);
    }
    else if (type.getCategory() == Typelib::Type::Numeric)
    {
        const Typelib::Numeric &numeric(static_cast<const Typelib::Numeric &>(valueIn.getType()));
        switch(numeric.getNumericCategory())
        {
            case Typelib::Numeric::Float:
                if(numeric.getSize() == sizeof(float))
                {
                    valueS = getValue<float>(valueIn);               
                }
                else
                {
                    valueS = getValue<double>(valueIn);               
                }
                break;
            case Typelib::Numeric::SInt:
                switch(numeric.getSize())
                {
                    case sizeof(int8_t):
                        valueS = getValue<int8_t>(valueIn);               
                        break;
                    case sizeof(int16_t):
                        valueS = getValue<int16_t>(valueIn);               
                        break;
                    case sizeof(int32_t):
                        valueS = getValue<int32_t>(valueIn);               
                        break;
                    case sizeof(int64_t):
                        valueS = getValue<int64_t>(valueIn);               
                        break;
                    default:
                        std::cout << "Error, got integer of unexpected size " << numeric.getSize() << std::endl;
                        return;
                }
                break;
            case Typelib::Numeric::UInt:
            {
                switch(numeric.getSize())
                {
                    case sizeof(uint8_t):
                        valueS = getValue<uint8_t>(valueIn);               
                        break;
                    case sizeof(uint16_t):
                        valueS = getValue<uint16_t>(valueIn);               
                        break;
                    case sizeof(uint32_t):
                        valueS = getValue<uint32_t>(valueIn);               
                        break;
                    case sizeof(uint64_t):
                        valueS = getValue<uint64_t>(valueIn);               
                        break;
                    default:
                        std::cout << "Error, got integer of unexpected size " << numeric.getSize() << std::endl;
                        return;
                }
            }
                break;
            case Typelib::Numeric::NumberOfValidCategories:
                std::cout << "Internal Error: Got invalid Category" << std::endl;
                return;
        }
    }
    else
    {
        std::cout << "got unsupported type.." << std::endl;
        return;
    }
    
    if (valueS != oldValue)
    {
        QTextCodec::ConverterState state;
        QTextCodec *codec = QTextCodec::codecForName("UTF-8");
        const QString text = codec->toUnicode(valueS.c_str(), valueS.size(), &state);
        if (state.invalidChars > 0)
        {
            return;
        }
        
        value->setText(valueS.c_str());
    }
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

bool Complex::hasActiveVisualizers()
{
    if (ItemBase::hasActiveVisualizers())
    {
        return true;
    }
    
    for (auto child: childs)
    {
        if (child->hasActiveVisualizers())
        {
            return true;
        }
    }
    
    return false;
}

void Complex::update(Typelib::Value& valueIn)
{
    const Typelib::Type &type(valueIn.getType());
    
    for (auto vizHandle : activeVizualizer)
    {   
        QGenericArgument data("void *", valueIn.getData());
        vizHandle.second.method.invoke(vizHandle.second.plugin, data);
    }
    
    
    if (type.getCategory() == Typelib::Type::Compound)
    {
        const Typelib::Compound &comp = static_cast<const Typelib::Compound &>(valueIn.getType());
     
        uint8_t *data = static_cast<uint8_t *>(valueIn.getData());
        
        size_t i = 0;
        for (const Typelib::Field &field: comp.getFields())
        {
            Typelib::Value fieldV(data + field.getOffset(), field.getType());
            
            if (childs.size() <= i)
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
            if (!content)
            {
                return;
            }
            
            QTextCodec::ConverterState state;
            QTextCodec *codec = QTextCodec::codecForName("UTF-8");
            const QString text = codec->toUnicode(content->c_str(), content->size(), &state);
            if (state.invalidChars > 0)
            {
                return;
            }
            
            value->setText(content->c_str());
            return;
        }
        
        //std::vector
        int numElemsShown = size;
        if (numElemsShown > maxVectorElemsShown)
        {
            numElemsShown = maxVectorElemsShown;
            value->setText(std::string(valueIn.getType().getName() + std::string(" [") + std::to_string(numElemsShown)
                + std::string(" of ") + std::to_string(size) + std::string(" elements displayed]") ).c_str());
        }
        else
        {
            value->setText(std::string(valueIn.getType().getName() + std::string(" [") + std::to_string(size) + std::string(" elements]")).c_str());
        }
        
        for(size_t i = 0; i < childs.size(); i++)
        {
            Typelib::Value elem = cont.getElement(valueIn.getData(), i);
            childs[i]->update(elem);
        }
        
        for(size_t i = childs.size(); i < numElemsShown; i++)
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
        case Typelib::Type::NullType:
        case Typelib::Type::Pointer:
        case Typelib::Type::Opaque:
        case Typelib::Type::NumberOfValidCategories:
            break;
    }

    throw std::runtime_error("Internal Error");
}

void ItemBase::addPlugin(std::pair<std::string, VizHandle> handle)
{
    activeVizualizer[handle.first] = handle.second;
}
#include "ConfigItem.hpp"
#include <rtt/plugin/PluginLoader.hpp>
#include <rtt/types/TypeInfoRepository.hpp>
#include <orocos_cpp_base/OrocosHelpers.hpp>
#include <boost/lexical_cast.hpp>
#include <rtt/typelib/TypelibMarshallerBase.hpp>
#include <rtt/typelib/TypelibMarshallerHandle.hpp>
#include <rtt/base/InputPortInterface.hpp>
#include <rtt/base/OutputPortInterface.hpp>
#include <rtt/types/TypeInfo.hpp>
#include <lib_config/TypelibConfiguration.hpp>
#include "Types.hpp"
#include <base-logging/Logging.hpp>
#include <boost/regex.hpp>

void VisualizerAdapter::addPlugin(const std::string &name, VizHandle handle)
{
    visualizers.insert(std::make_pair(name, handle));
}

QObject* VisualizerAdapter::getVisualizer(const std::string& name)
{
    std::map<std::string, VizHandle>::iterator iter = visualizers.find(name);
    if (iter != visualizers.end())
    {
        return iter->second.plugin;
    }
    
    return nullptr;
}

bool VisualizerAdapter::hasVisualizer(const std::string& name)
{
    return visualizers.find(name) != visualizers.end();
}

bool VisualizerAdapter::removeVisualizer(QObject* plugin)
{
    for (std::map<std::string, VizHandle>::iterator it = visualizers.begin(); it != visualizers.end(); it++)
    {
        if (it->second.plugin == plugin)
        {
            visualizers.erase(it);
            return true;
        }
    }
    
    return false;
}

ItemBase::ItemBase() : VisualizerAdapter(), name(new TypedItem()), value(new TypedItem()), codec(QTextCodec::codecForName("UTF-8"))
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
    if (!visualizers.empty())
    {
        return true;
    }
    
    for (auto child: children)
    {
        if (child->hasActiveVisualizers())
        {
            return true;
        }
    }
    
    return false;
}

Array::Array(Typelib::Value& valueIn)
    : ItemBase()
{
    const Typelib::Array &array = static_cast<const Typelib::Array &>(valueIn.getType());
    value->setText(array.getName().c_str());
    update(valueIn, true, true);
}

Array::~Array()
{

}

bool Array::update(Typelib::Value& valueIn, bool updateUI, bool forceUpdate)
{    
    bool updateNecessary = (this->name->isExpanded() && updateUI) | forceUpdate;
    const Typelib::Array &array = static_cast<const Typelib::Array &>(valueIn.getType());
    
    void *data = valueIn.getData();
    
    const Typelib::Type &indirect(array.getIndirection());
    
    size_t numElemsToDisplay = array.getDimension();
    if (numElemsToDisplay > maxArrayElemsDisplayed)
    {
        numElemsToDisplay = maxArrayElemsDisplayed;
    }
    
    if (name->rowCount() < 0)
    {
        return false;
    }
    size_t currentRows = name->rowCount();
    
    bool numElemsDisplayedChanged = (numElemsToDisplay != currentRows);
    
    if (currentRows > numElemsToDisplay)
    {
        while (name->rowCount() > 0)
        {   
            name->takeRow(0);
        }
        
        currentRows = 0;
    }
    
    bool childRet = false;
    for (size_t i = 0; i < std::min(currentRows, numElemsToDisplay); i++)
    {
        Typelib::Value arrayV(static_cast<uint8_t *>(data) + i * indirect.getSize(), indirect);
        childRet |= children[i]->update(arrayV, updateNecessary);
    }
    updateNecessary &= childRet;
    
    for (size_t i = currentRows; i < numElemsToDisplay; i++)
    {
        Typelib::Value arrayV(static_cast<uint8_t *>(data) + i * indirect.getSize(), indirect);
        std::shared_ptr<ItemBase> child = nullptr;
            
        if (children.size() > i)
        {
            children[i]->update(arrayV, true);
            child = children[i];
        }
        else
        {
            child = getItem(arrayV);
            children.push_back(child);
        }
        
        child->setName(QString::number(i));
        name->appendRow(child->getRow());
    }
    
    return updateNecessary || numElemsDisplayedChanged;
}

Simple::Simple(Typelib::Value& valueIn)
    : ItemBase()
{
    name->setText(valueIn.getType().getName().c_str());   
    update(valueIn, true, true);
}

Simple::~Simple()
{

}

template <class T>
std::string getValue(const Typelib::Value& value)
{
    T *val = static_cast<T *>(value.getData());
    std::string valueS = "";
    
    if (!val)
    {
        return valueS;
    }
    
    try
    {
        valueS = boost::lexical_cast<std::string>(*val);
    }
    catch (...)
    {
        
    }
    
    return valueS;
}

bool Simple::update(Typelib::Value& valueIn, bool updateUI, bool forceUpdate)
{    
    bool updateNecessary = updateUI || forceUpdate;
    
    if (!updateUI)
    {
        return false;
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
                        LOG_ERROR_S << "Error, got integer of unexpected size " << numeric.getSize();
                        return false;
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
                        LOG_ERROR_S << "Error, got integer of unexpected size " << numeric.getSize();
                        return false;
                }
            }
                break;
            case Typelib::Numeric::NumberOfValidCategories:
                LOG_ERROR_S << "Internal Error: Got invalid Category";
                return false;
        }
    }
    else
    {
        LOG_WARN_S << "got unsupported type..";
        return false;
    }
    
    if (valueS != oldValue)
    {
        if (codec)
        {
            const QString text = codec->toUnicode(valueS.c_str(), valueS.size(), &state);
            
            if (state.invalidChars > 0)
            {
                return updateNecessary;
            }
            
            value->setText(text);
            return true;
        }
    }
    
    return false;
}

Complex::Complex(Typelib::Value& valueIn)
    : ItemBase()
{
    transport = nullptr;
    transportHandle = nullptr;
    
    const Typelib::Type &type(valueIn.getType());
    RTT::types::TypeInfoRepository::shared_ptr ti = RTT::types::TypeInfoRepository::Instance();
    std::string typeStr = type.getName();
    
    typeStr = boost::regex_replace(typeStr, boost::regex("_m"), "");
    
    RTT::types::TypeInfo* typeInfo = ti->type(typeStr);
    if (typeInfo)
    {
        transport = dynamic_cast<orogen_transports::TypelibMarshallerBase*>(typeInfo->getProtocol(orogen_transports::TYPELIB_MARSHALLER_ID));
        transportHandle = transport->createSample();
        sample = transport->getDataSource(transportHandle);
    }
    else
    {
        LOG_WARN_S << "cannot find " << typeStr << " in the type info repository";
    }  
    
    update(valueIn, true, true);
    value->setText(valueIn.getType().getName().c_str());
}

Complex::~Complex()
{

}

bool Complex::update(Typelib::Value& valueIn, bool updateUI, bool forceUpdate)
{   
    bool updateNecessary = (updateUI && this->name->isExpanded()) || forceUpdate;
    const Typelib::Type &type(valueIn.getType());
    
    if (!visualizers.empty() && transport)
    {
        transport->setTypelibSample(transportHandle, valueIn);
        sample = transport->getDataSource(transportHandle);
    
        for (auto vizHandle : visualizers)
        {   
            QGenericArgument data("void *", sample.get()->getRawPointer());
            vizHandle.second.method.invoke(vizHandle.second.plugin, data);
        }
    }
    
    if (type.getCategory() == Typelib::Type::Compound)
    {
        const Typelib::Compound &comp = static_cast<const Typelib::Compound &>(type);
        
     
        uint8_t *data = static_cast<uint8_t *>(valueIn.getData());
        
        bool childRet = false;
        size_t i = 0;
        for (const Typelib::Field &field: comp.getFields())
        {
            Typelib::Value fieldV(data + field.getOffset(), field.getType());
            
            if (children.size() <= i)
            {
                std::shared_ptr<ItemBase> newVal = getItem(fieldV);
                newVal->setName(field.getName().c_str());
                children.push_back(newVal);
                name->appendRow(newVal->getRow());
                childRet = true;
                i++;
                continue;
            }

            childRet |= children[i]->update(fieldV, updateNecessary);           
            i++;
        }
        updateNecessary &= childRet;
    }
    else
    {
        const Typelib::Container &cont = static_cast<const Typelib::Container &>(valueIn.getType());
        const size_t size = cont.getElementCount(valueIn.getData());
        
        if(cont.kind() == "/std/string")
        {   
            if (!updateUI)
            {
                return false;
            }
            
            const std::string content = *static_cast<const std::string *>(valueIn.getData());
            
            if (codec)
            {
                const QString text = codec->toUnicode(content.c_str(), content.size(), &state);
                if (state.invalidChars > 0)
                {
                    return updateNecessary;
                }
                
                if (value->text().toStdString() != text.toStdString())
                {
                    value->setText(text);
                    return true;
                }
            }
            
            return updateNecessary;
        }
        
        //std::vector
        size_t numElemsToDisplay = size;
        bool areMaxElemsDiplayed = false;
        // limit max elems displayed to maxVectorElemsDisplayed
        if (numElemsToDisplay > maxVectorElemsDisplayed)
        {
            numElemsToDisplay = maxVectorElemsDisplayed;
            areMaxElemsDiplayed = true;
        }
        
        if (name->rowCount())
        {
            return false;
        }
        size_t currentRows = name->rowCount();
        
        // check if number of displayed items changed
        bool numElemsDisplayedChanged = (numElemsToDisplay != currentRows);
        
        if (currentRows > numElemsToDisplay)
        {
            while (name->rowCount() > 0)
            {   
                name->takeRow(0);
            }
            
            currentRows = 0;
        } 
        
        bool childRet = false;
        // update old (displayed) items
        // children size at this step is min(currentRows, numElemsToDisplay)
        for(size_t i = 0; i < std::min(currentRows, numElemsToDisplay); i++)
        {
            Typelib::Value elem = cont.getElement(valueIn.getData(), i);
            childRet |= children[i]->update(elem, updateNecessary);
        }
        updateNecessary &= childRet; 
        
        // case new vector size is bigger
        // append new rows
        for(size_t i = currentRows; i < numElemsToDisplay; i++)
        {
            Typelib::Value elem = cont.getElement(valueIn.getData(), i);
            std::shared_ptr<ItemBase> child = nullptr;
            
            if (children.size() > i)
            {
                children[i]->update(elem, true);
                child = children[i];
            }
            else
            {
                child = getItem(elem);
                children.push_back(child);
            }
            
            child->setName(QString::number(i));
            name->appendRow(child->getRow());
        }  
        
        if (numElemsDisplayedChanged)
        {
            if (areMaxElemsDiplayed)
            {
                value->setText(std::string(valueIn.getType().getName() + std::string(" [") + std::to_string(numElemsToDisplay)
                    + std::string(" of ") + std::to_string(size) + std::string(" elements displayed]") ).c_str());
            }
            else
            {
                value->setText(std::string(valueIn.getType().getName() + std::string(" [") + std::to_string(size) + std::string(" elements]")).c_str());
            }
        }
        
        updateNecessary |= numElemsDisplayedChanged;
    }
    
    return updateNecessary;
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

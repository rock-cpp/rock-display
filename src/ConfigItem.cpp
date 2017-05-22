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
#include <orocos_cpp/TypeRegistry.hpp>
#include <orocos_cpp/PkgConfigHelper.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <fstream>
#include <tinyxml.h>

std::map<std::string, std::string> ItemBase::lookupMarshalledTypelistTypes()
{
    std::map<std::string, std::string> marshalled2Typelib;
    const char *pkgCfgPaths = getenv("PKG_CONFIG_PATH");
    
    if(!pkgCfgPaths)
    {
        return marshalled2Typelib;
    }
    
    std::vector<std::string> paths;
    boost::split(paths, pkgCfgPaths, boost::is_any_of(":"));

    for(const std::string& path : paths)
    {
        if(!boost::filesystem::exists(boost::filesystem::path(path)))
        {
            LOG_WARN_S << "skipping nonexisting pkg-config path: " << path;
            continue;
        }
        
        for(auto it = boost::filesystem::directory_iterator(path); it != boost::filesystem::directory_iterator(); it++)
        {
            const boost::filesystem::path file = it->path();
            
            if(file.filename().string().find("typekit") != std::string::npos)
            {                
                // be aware of order of parsed fields
                std::vector<std::string> result, fields{"prefix", "project_name", "type_registry"};
                if(orocos_cpp::PkgConfigHelper::parsePkgConfig(file.filename().string(), fields, result))
                {
                    if(orocos_cpp::PkgConfigHelper::solveString(result.at(2), "${prefix}", result.at(0)))
                    {
                        std::string typeKitPath = result.at(2);
                        
                        std::ifstream in(typeKitPath);
                        if(in.bad())
                        {
                            throw std::runtime_error("ItemBase::lookupMarshalledTypelistTypes(): failed to open file " + file.filename().string());
                        }
                        
                        TiXmlDocument doc(typeKitPath.c_str());
                        if (!doc.LoadFile())
                        {
                            LOG_WARN_S << "ItemBase::lookupMarshalledTypelistTypes(): tinyxml could not load " << typeKitPath;
                            continue;
                        }
                        
                        TiXmlElement* typelibElem = doc.FirstChildElement("typelib");
                        if (!typelibElem)
                        {
                            throw std::runtime_error("ItemBase::lookupMarshalledTypelistTypes(): no 'typelib' element in " + typeKitPath);
                        }
                        
                        for (TiXmlElement* sub = typelibElem->FirstChildElement("opaque"); sub; sub = sub->NextSiblingElement("opaque"))
                        {
                            std::string marshalAs;
                            std::string name;
                            sub->QueryStringAttribute("marshal_as", &marshalAs);
                            sub->QueryStringAttribute("name", &name);
                            LOG_INFO_S << "type " << name << " marshalled as " << marshalAs;
                            marshalled2Typelib[marshalAs] = name;
                        }
                    }
                    else
                    {
                        LOG_WARN_S << "error: couldn't solve pkg-config strings from file " << file.string();
                    }
                }
                else
                {
                    LOG_WARN_S << "error: couldn't parse pkg-config fields from file " << file.string();
                }   
            }   
        }
    }
    
    return marshalled2Typelib;
}

std::map<std::string, std::string> ItemBase::marshalled2Typelib = lookupMarshalledTypelistTypes();

void VisualizerAdapter::addPlugin(const std::string &name, VizHandle handle)
{
    LOG_INFO_S << "add plugin " << name << "..";
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

ItemBase::ItemBase()
    : VisualizerAdapter(),
      name(new TypedItem()),
      value(new TypedItem()),
      codec(QTextCodec::codecForName("UTF-8"))
{
    name->setType(ItemType::CONFIGITEM);
    value->setType(ItemType::CONFIGITEM);
    name->setData(this);
    value->setData(this);
}

ItemBase::ItemBase(TypedItem *name, TypedItem *value)
    : VisualizerAdapter(),
      name(name), value(value),
      codec(QTextCodec::codecForName("UTF-8"))
{
    if (!name || !value)
    {
        this->name = new TypedItem();
        this->value = new TypedItem();
        
        this->name->setType(ItemType::CONFIGITEM);
        this->value->setType(ItemType::CONFIGITEM);
        this->name->setData(this);
        this->value->setData(this);
    }
}

ItemBase::~ItemBase()
{
}

bool ItemBase::hasVisualizers()
{
    std::cout << "ItemBase::hasVisualizers().." << std::endl;
    if (!visualizers.empty())
    {
        return true;
    }
    
    for (auto child: children)
    {
        if (child->hasVisualizers())
        {
            return true;
        }
    }
    
    return false;
}

Array::Array(Typelib::Value& valueIn, TypedItem *name, TypedItem *value)
    : ItemBase(name, value)
{
    const Typelib::Array &array = static_cast<const Typelib::Array &>(valueIn.getType());
    this->value->setText(array.getName().c_str());
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
    
    int numElemsToDisplay = static_cast<int>(array.getDimension());
    if (numElemsToDisplay > maxArrayElemsDisplayed)
    {
        numElemsToDisplay = maxArrayElemsDisplayed;
    }
    
    if (name->rowCount() < 0)
    {
        return false;
    }
    int currentRows = name->rowCount();
    
    bool numElemsDisplayedChanged = (numElemsToDisplay != currentRows);
    
    if (currentRows > numElemsToDisplay)
    {
        while (name->rowCount() > numElemsToDisplay)
        {   
            name->takeRow(name->rowCount() -1);
        }
        
        currentRows = numElemsToDisplay;
    }
    
    bool childRet = false;
    for (int i = 0; i < std::min(currentRows, numElemsToDisplay); i++)
    {
        Typelib::Value arrayV(static_cast<uint8_t *>(data) + i * indirect.getSize(), indirect);
        childRet |= children[i]->update(arrayV, updateNecessary);
    }
    updateNecessary &= childRet;
    
    for (int i = currentRows; i < numElemsToDisplay; i++)
    {
        Typelib::Value arrayV(static_cast<uint8_t *>(data) + i * indirect.getSize(), indirect);
        std::shared_ptr<ItemBase> child = nullptr;
            
        if (static_cast<int>(children.size()) > i)
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

Simple::Simple(Typelib::Value& valueIn, TypedItem *name, TypedItem *value)
    : ItemBase(name, value)
{
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

Complex::Complex(Typelib::Value& valueIn, TypedItem *name, TypedItem *value)
    : ItemBase(name, value)
{
    transport = nullptr;
    transportHandle = nullptr;
    
    const Typelib::Type &type(valueIn.getType());
    RTT::types::TypeInfoRepository::shared_ptr ti = RTT::types::TypeInfoRepository::Instance();
    std::string typeStr = type.getName();
    
    // HACK: resolve typelib type for marshalling types ending with '_m'
    if (boost::algorithm::ends_with(typeStr, "_m"))
    {
        marshalled2Typelib[typeStr] = typeStr.substr(0, typeStr.size()-2);
    }
    
    if (marshalled2Typelib.find(typeStr) != marshalled2Typelib.end())
    {
        LOG_INFO_S << "lookup type " << marshalled2Typelib[typeStr] << " for marshalled type " << typeStr;
        typeStr = marshalled2Typelib[typeStr];
    }
    
    RTT::types::TypeInfo* typeInfo = ti->type(typeStr);
    if (typeInfo)
    {
        transport = dynamic_cast<orogen_transports::TypelibMarshallerBase*>(typeInfo->getProtocol(orogen_transports::TYPELIB_MARSHALLER_ID));
        transportHandle = transport->createSample();
        sample = transport->getDataSource(transportHandle);
    }
    else
    {
        LOG_WARN_S << "typeInfo for " << typeStr << " unknown..";
    }
    
    update(valueIn, true, true);
    this->value->setText(valueIn.getType().getName().c_str());
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
            if (!forceUpdate && !updateUI)
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
        int numElemsToDisplay = static_cast<int>(size);
        bool areMaxElemsDiplayed = false;
        // limit max elems displayed to maxVectorElemsDisplayed
        if (numElemsToDisplay > maxVectorElemsDisplayed)
        {
            numElemsToDisplay = maxVectorElemsDisplayed;
            areMaxElemsDiplayed = true;
        }
        
        int currentRows = name->rowCount();
        
        // check if number of displayed items changed
        bool numElemsDisplayedChanged = (numElemsToDisplay != currentRows);
        
        if (currentRows > numElemsToDisplay)
        {
            while (name->rowCount() > numElemsToDisplay)
            {   
                name->takeRow(name->rowCount() -1);
            }
            
            currentRows = numElemsToDisplay;
        }
        
        bool childRet = false;
        // update old (displayed) items
        // children size at this step is min(currentRows, numElemsToDisplay)
        for(int i = 0; i < std::min(currentRows, numElemsToDisplay); i++)
        {
            Typelib::Value elem = cont.getElement(valueIn.getData(), i);
            childRet |= children[i]->update(elem, updateNecessary);
        }
        updateNecessary &= childRet; 
        
        // case new vector size is bigger
        // append new rows
        for(int i = currentRows; i < numElemsToDisplay; i++)
        {
            Typelib::Value elem = cont.getElement(valueIn.getData(), i);
            std::shared_ptr<ItemBase> child = nullptr;
            
            if (static_cast<int>(children.size()) > i)
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

std::shared_ptr< ItemBase > getItem(Typelib::Value& value, TypedItem *nameItem, TypedItem *valueItem)
{   
    const Typelib::Type &type(value.getType());
    
    switch(type.getCategory())
    {
        case Typelib::Type::Array:
            return std::shared_ptr<ItemBase>(new Array(value, nameItem, valueItem));
            break;
        case Typelib::Type::Enum:
        case Typelib::Type::Numeric:
            return std::shared_ptr<ItemBase>(new Simple(value, nameItem, valueItem));
            break;
        case Typelib::Type::Compound:
        case Typelib::Type::Container:
            return std::shared_ptr<ItemBase>(new Complex(value, nameItem, valueItem));
            break;
        case Typelib::Type::NullType:
        case Typelib::Type::Pointer:
        case Typelib::Type::Opaque:
        case Typelib::Type::NumberOfValidCategories:
            break;
    }

    throw std::runtime_error("Internal Error");
}
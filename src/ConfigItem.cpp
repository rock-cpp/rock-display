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
#include "ConfigItemHandlerRepository.hpp"

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
    if (VisualizerAdapter::hasVisualizers())
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

Typelib::Value& ItemBase::getValueHandle()
{
    static Typelib::Value novalue;
    return novalue;
}

Array::Array(TypedItem *name, TypedItem *value)
    : ItemBase(name, value)
{
}

Array::~Array()
{

}

std::shared_ptr<ItemBase> Array::getItem(Typelib::Value& value, TypedItem *nameItem, TypedItem *valueItem) const
{
    return ::getItem(value, handlerrepo, nameItem, valueItem);
}

bool Array::update(Typelib::Value& valueIn, bool updateUI, bool forceUpdate)
{    
    bool updateNecessary = this->name->isExpanded() && updateUI;
    const Typelib::Array &array = static_cast<const Typelib::Array &>(valueIn.getType());
    this->value->setText(array.getName().c_str());
    
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
        childRet |= children[i]->update(arrayV, updateNecessary, forceUpdate);
    }
    updateNecessary &= childRet;
    
    for (int i = currentRows; i < numElemsToDisplay; i++)
    {
        Typelib::Value arrayV(static_cast<uint8_t *>(data) + i * indirect.getSize(), indirect);
        std::shared_ptr<ItemBase> child = nullptr;
            
        if (static_cast<int>(children.size()) > i)
        {
            child = children[i];
        }
        else
        {
            child = getItem(arrayV);
            children.push_back(child);
        }

        child->update(arrayV, true, forceUpdate);
        child->setName(QString::number(i));
        name->appendRow(child->getRow());
    }
    
    return forceUpdate || updateNecessary || numElemsDisplayedChanged;
}

EditableArray::EditableArray(Typelib::Value& valueIn, TypedItem *name, TypedItem *value)
    : Array(name, value)
    , value_handle(valueIn)
{
    if(!name || !value)
    {
        this->name->setType(ItemType::EDITABLEITEM);
        this->value->setType(ItemType::EDITABLEITEM);
    }
}

EditableArray::~EditableArray()
{

}

Typelib::Value& EditableArray::getValueHandle()
{
    return value_handle;
}

std::shared_ptr<ItemBase> EditableArray::getItem(Typelib::Value& value, TypedItem *nameItem, TypedItem *valueItem) const
{
    return ::getEditableItem(value, handlerrepo, nameItem, valueItem);
}

bool EditableArray::update(Typelib::Value& valueIn, bool updateUI, bool forceUpdate)
{
    value_handle = valueIn;
    return Array::update(valueIn, updateUI, forceUpdate);
}

bool EditableArray::compareAndMark ( Typelib::Value& valueCurrent, Typelib::Value& valueOld )
{
    bool isEqual = true;

    const Typelib::Array &array = static_cast<const Typelib::Array &>(valueCurrent.getType());
    this->value->setText(array.getName().c_str());

    void *dataCurrent = valueCurrent.getData();
    void *dataOld = valueOld.getData();

    const Typelib::Type &indirect(array.getIndirection());

    size_t numElems = array.getDimension();

    if (children.size() < numElems)
    {
        isEqual &= Typelib::compare(valueCurrent, valueOld);
    }

    for (size_t i = 0; i < children.size() && i < numElems; i++)
    {
        Typelib::Value arrayVCurrent(static_cast<uint8_t *>(dataCurrent) + i * indirect.getSize(), indirect);
        Typelib::Value arrayVOld(static_cast<uint8_t *>(dataOld) + i * indirect.getSize(), indirect);
        isEqual &= children[i]->compareAndMark(arrayVCurrent, arrayVOld);
    }

    name->QStandardItem::setData(QVariant(!isEqual), ItemBase::ModifiedRole);
    value->QStandardItem::setData(QVariant(!isEqual), ItemBase::ModifiedRole);

    return isEqual;
}

Simple::Simple(TypedItem *name, TypedItem *value)
    : ItemBase(name, value)
{
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

template <>
std::string getValue<uint8_t>(const Typelib::Value& value)
{
    uint8_t *val = static_cast<uint8_t *>(value.getData());
    std::string valueS = "";

    if (!val)
    {
        return valueS;
    }

    try
    {
        valueS = boost::lexical_cast<std::string>((int)*val);
    }
    catch (...)
    {

    }

    return valueS;
}

template <>
std::string getValue<int8_t>(const Typelib::Value& value)
{
    int8_t *val = static_cast<int8_t *>(value.getData());
    std::string valueS = "";

    if (!val)
    {
        return valueS;
    }

    try
    {
        valueS = boost::lexical_cast<std::string>((int)*val);
    }
    catch (...)
    {

    }

    return valueS;
}

bool Simple::update(Typelib::Value& valueIn, bool updateUI, bool forceUpdate)
{
    bool updateNecessary = forceUpdate || updateUI;
    
    if (!updateNecessary)
    {
        return false;
    }
    
    const Typelib::Type &type(valueIn.getType());
    std::string valueS = value->text().toStdString();
    if (codec)
    {
        QByteArray bytes = codec->fromUnicode(value->text());
        valueS = bytes.toStdString();
    }
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
        if (numeric.getName() == "/bool")
        {
            bool *boolVal = static_cast<bool *>(valueIn.getData());
            if (boolVal)
            {
                valueS = (*boolVal ? "true" : "false");
            }
        }
        else
        {
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
            QTextCodec::ConverterState state;
            const QString text = codec->toUnicode(valueS.c_str(), valueS.size(), &state);
            
            if (state.invalidChars > 0)
            {
                return updateNecessary;
            }
            
            value->setText(text);
            return true;
        }
    }
    
    return updateNecessary;
}

EditableSimple::EditableSimple(Typelib::Value& valueIn, TypedItem *name, TypedItem *value)
    : Simple(name, value)
    , value_handle(valueIn)
{
    if(this->value)
        this->value->setEditable(true);
    if(!name || !value)
    {
        this->name->setType(ItemType::EDITABLEITEM);
        this->value->setType(ItemType::EDITABLEITEM);
    }
}

EditableSimple::~EditableSimple()
{

}

Typelib::Value& EditableSimple::getValueHandle()
{
    return value_handle;
}

bool EditableSimple::update(Typelib::Value& valueIn, bool updateUI, bool forceUpdate)
{
    value_handle = valueIn;
    return Simple::update(valueIn, updateUI, forceUpdate);
}

bool EditableSimple::compareAndMark ( Typelib::Value& valueCurrent, Typelib::Value& valueOld )
{
    bool isEqual = Typelib::compare(valueCurrent, valueOld);

    name->QStandardItem::setData(QVariant(!isEqual), ItemBase::ModifiedRole);
    value->QStandardItem::setData(QVariant(!isEqual), ItemBase::ModifiedRole);

    return isEqual;
}

void Complex::addPlugin(const std::string& name, VizHandle handle)
{
    VisualizerAdapter::addPlugin(name, handle);
    
    if (!transport)
    {
        RTT::types::TypeInfoRepository::shared_ptr ti = RTT::types::TypeInfoRepository::Instance();
        std::string typeStr = typelibType.getName();
        
        // HACK: resolve typelib type for marshalling types ending with '_m'
        if (boost::algorithm::ends_with(typeStr, "_m"))
        {
            marshalled2Typelib[typeStr] = typeStr.substr(0, typeStr.size()-2);
        }
        else if (boost::algorithm::ends_with(typeStr, "_m>"))
        {
            marshalled2Typelib[typeStr] = typeStr.substr(0, typeStr.size()-3) + ">";
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
    }
}

Complex::Complex(Typelib::Value& valueIn, TypedItem *name, TypedItem *value)
    : ItemBase(name, value),
      typelibType(valueIn.getType())
{
    transport = nullptr;
    transportHandle = nullptr;
}

Complex::~Complex()
{

}

std::shared_ptr<ItemBase> Complex::getItem(Typelib::Value& value, TypedItem *nameItem, TypedItem *valueItem) const
{
    return ::getItem(value, handlerrepo, nameItem, valueItem);
}

bool Complex::update(Typelib::Value& valueIn, bool updateUI, bool forceUpdate)
{       
    bool updateNecessary = updateUI && this->name->isExpanded();
    const Typelib::Type &type(valueIn.getType());

    this->value->setText(valueIn.getType().getName().c_str());

    if (!visualizers.empty() && transport)
    {           
        transport->setTypelibSample(transportHandle, valueIn);
        for (auto vizHandle : visualizers)
        {
            updateVisualizer(vizHandle.second, sample);
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
            }

            childRet |= children[i]->update(fieldV, updateNecessary, forceUpdate);           
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
            if (!forceUpdate && !updateNecessary)
            {
                return false;
            }
            
            const std::string content = *static_cast<const std::string *>(valueIn.getData());
            
            if (codec)
            {
                QTextCodec::ConverterState state;
                const QString text = codec->toUnicode(content.c_str(), content.size(), &state);
                if (state.invalidChars > 0)
                {
                    return updateNecessary;
                }
                else if (value->text().toStdString() != text.toStdString())
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
            childRet |= children[i]->update(elem, updateNecessary, forceUpdate);
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
                child = children[i];
            }
            else
            {
                child = getItem(elem);
                children.push_back(child);
            }

            child->update(elem, true, forceUpdate);
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
    
    return forceUpdate || updateNecessary;
}

EditableComplex::EditableComplex(Typelib::Value& valueIn, TypedItem *name, TypedItem *value)
    : Complex(valueIn, name, value)
    , value_handle(valueIn)
{
    if(!name || !value)
    {
        this->name->setType(ItemType::EDITABLEITEM);
        this->value->setType(ItemType::EDITABLEITEM);
    }
}

EditableComplex::~EditableComplex()
{
}

Typelib::Value& EditableComplex::getValueHandle()
{
    return value_handle;
}


std::shared_ptr<ItemBase> EditableComplex::getItem(Typelib::Value& value, TypedItem *nameItem, TypedItem *valueItem) const
{
    return ::getEditableItem(value, handlerrepo, nameItem, valueItem);
}

bool EditableComplex::update(Typelib::Value& valueIn, bool updateUI, bool forceUpdate)
{
    value_handle = valueIn;
    return Complex::update(valueIn, updateUI, forceUpdate);
}

bool EditableComplex::compareAndMark ( Typelib::Value& valueCurrent, Typelib::Value& valueOld )
{
    const Typelib::Type &type(valueCurrent.getType());
    bool isEqual = true;

    if (type.getCategory() == Typelib::Type::Compound)
    {

        const Typelib::Compound &comp = static_cast<const Typelib::Compound &>(type);
        uint8_t *dataCurrent = static_cast<uint8_t *>(valueCurrent.getData());
        uint8_t *dataOld = static_cast<uint8_t *>(valueOld.getData());
        size_t i = 0;
        if(children.size() < comp.getFields().size())
        {
            isEqual &= Typelib::compare(valueCurrent, valueOld);
        }
        for (const Typelib::Field &field: comp.getFields())
        {
            Typelib::Value fieldVCurrent(dataCurrent + field.getOffset(), field.getType());
            Typelib::Value fieldVOld(dataOld + field.getOffset(), field.getType());

            if (children.size() <= i)
            {
                break;
            }

            isEqual &= children[i]->compareAndMark(fieldVCurrent, fieldVOld);
            i++;
        }
    }
    else
    {
        const Typelib::Container &cont = static_cast<const Typelib::Container &>(valueCurrent.getType());
        const size_t sizeCurrent = cont.getElementCount(valueCurrent.getData());
        const size_t sizeOld = cont.getElementCount(valueOld.getData());

        isEqual &= sizeOld == sizeCurrent;

        if (children.size() < sizeCurrent &&
            children.size() < sizeOld)
        {
            isEqual &= Typelib::compare(valueCurrent, valueOld);
        }
        for(size_t i = 0; i < children.size() && i < sizeCurrent && i < sizeOld; i++)
        {
            Typelib::Value elemCurrent = cont.getElement(valueCurrent.getData(), i);
            Typelib::Value elemOld = cont.getElement(valueOld.getData(), i);

            children[i]->compareAndMark(elemCurrent,elemOld);
        }
    }

    name->QStandardItem::setData(QVariant(!isEqual), ItemBase::ModifiedRole);
    value->QStandardItem::setData(QVariant(!isEqual), ItemBase::ModifiedRole);

    return isEqual;
}

std::shared_ptr< ItemBase > getItem(Typelib::Value& value, ConfigItemHandlerRepository *handlerrepo, TypedItem *nameItem, TypedItem *valueItem)
{   
    const Typelib::Type &type(value.getType());
    
    std::shared_ptr<ItemBase> itembase;

    switch(type.getCategory())
    {
        case Typelib::Type::Array:
            itembase = std::shared_ptr<ItemBase>(new Array(nameItem, valueItem));
            break;
        case Typelib::Type::Enum:
        case Typelib::Type::Numeric:
            itembase = std::shared_ptr<ItemBase>(new Simple(nameItem, valueItem));
            break;
        case Typelib::Type::Compound:
        case Typelib::Type::Container:
            itembase = std::shared_ptr<ItemBase>(new Complex(value, nameItem, valueItem));
            break;
        case Typelib::Type::NullType:
        case Typelib::Type::Pointer:
        case Typelib::Type::Opaque:
        case Typelib::Type::NumberOfValidCategories:
            break;
    }

    if(!itembase)
        throw std::runtime_error("Internal Error");

    itembase->setHandlerRepo(handlerrepo);
    return itembase;
}

std::shared_ptr< ItemBase > getEditableItem(Typelib::Value& value, ConfigItemHandlerRepository *handlerrepo, TypedItem *nameItem, TypedItem *valueItem)
{
    const Typelib::Type &type(value.getType());

    std::shared_ptr<ItemBase> itembase;

    switch(type.getCategory())
    {
        case Typelib::Type::Array:
            itembase = std::shared_ptr<ItemBase>(new EditableArray(value, nameItem, valueItem));
            break;
        case Typelib::Type::Enum:
        case Typelib::Type::Numeric:
            itembase = std::shared_ptr<ItemBase>(new EditableSimple(value, nameItem, valueItem));
            break;
        case Typelib::Type::Compound:
        case Typelib::Type::Container:
            itembase = std::shared_ptr<ItemBase>(new EditableComplex(value, nameItem, valueItem));
            break;
        case Typelib::Type::NullType:
        case Typelib::Type::Pointer:
        case Typelib::Type::Opaque:
        case Typelib::Type::NumberOfValidCategories:
            break;
    }

    if(!itembase)
        throw std::runtime_error("Internal Error");

    itembase->setHandlerRepo(handlerrepo);
    return itembase;
}

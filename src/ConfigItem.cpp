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
#include <orocos_cpp/PkgConfigHelper.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <fstream>
#include <tinyxml.h>
#include "ConfigItemHandler.hpp"
#include "ConfigItemHandlerRepository.hpp"
#include <QTextCodec>
#include <QApplication>
#include <QThread>

ItemBase::ItemBase()
    : VisualizerAdapter(),
      name(new TypedItem()),
      value(new TypedItem()),
      handlerrepo(nullptr),
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
      handlerrepo(nullptr),
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

ssize_t ItemBase::indexOf(ItemBase *childitem) const {
    for(unsigned int i = 0; i < children.size(); i++) {
        if(children[i].get() == childitem) {
            return i;
        }
    }
    return -1;
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

void Array::update(Typelib::Value& valueIn, bool updateUI)
{    
    bool childrenUpdateUI = updateUI && name->isExpanded();
    value_handle = valueIn;

    emit visualizerUpdate(valueIn);

    for(auto &h : handlerStack)
    {
        if (updateUI && (h->flags() & ConfigItemHandler::Flags::ConvertsFromTypelibValue))
        {
            h->convertFromTypelibValue(value, valueIn, codec);
        }
        if (h->flags() & ConfigItemHandler::Flags::HideFields)
        {
            return;
        }
    }

    const Typelib::Array &array = static_cast<const Typelib::Array &>(valueIn.getType());
    if(updateUI)
    {
        this->value->setText(array.getName().c_str());
    }
    
    void *data = valueIn.getData();
    
    const Typelib::Type &indirect(array.getIndirection());
    
    int numElemsToDisplay = static_cast<int>(array.getDimension());
    if (numElemsToDisplay > maxArrayElemsDisplayed)
    {
        numElemsToDisplay = maxArrayElemsDisplayed;
    }
    
    if (name->rowCount() < 0)
    {
        return;
    }
    int currentRows = updateUI?name->rowCount():children.size();
    
    if (updateUI && currentRows > numElemsToDisplay)
    {
        while (name->rowCount() > numElemsToDisplay)
        {   
            name->takeRow(name->rowCount() -1);
        }
        
        currentRows = numElemsToDisplay;
    }
    
    for (int i = 0; i < std::min(currentRows, numElemsToDisplay); i++)
    {
        Typelib::Value arrayV(static_cast<uint8_t *>(data) + i * indirect.getSize(), indirect);
        children[i]->update(arrayV, childrenUpdateUI);
    }
    
    if (!updateUI)
    {
        return;
    }
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

        child->update(arrayV, childrenUpdateUI);
        child->setName(QString::number(i));
        name->appendRow(child->getRow());
    }
}

EditableArray::EditableArray(TypedItem *name, TypedItem *value)
    : Array(name, value)
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

void EditableArray::update(Typelib::Value& valueIn, bool updateUI)
{
    emit editableUpdate(valueIn);

    Array::update(valueIn, updateUI);
}

std::shared_ptr<ItemBase> EditableArray::getItem(Typelib::Value& value, TypedItem *nameItem, TypedItem *valueItem) const
{
    return ::getEditableItem(value, handlerrepo, nameItem, valueItem);
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

void Simple::update(Typelib::Value& valueIn, bool updateUI)
{
    value_handle = valueIn;

    emit visualizerUpdate(valueIn);

    if (!updateUI)
    {
        return;
    }

    for(auto &h : handlerStack)
    {
        if (h->flags() & ConfigItemHandler::Flags::ConvertsFromTypelibValue)
        {
            h->convertFromTypelibValue(value, valueIn, codec);
            return;
        }
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
                            LOG_ERROR_S << "Error, got integer of unexpected size " << numeric.getSize();
                            return;
                    }
                }
                    break;
                case Typelib::Numeric::NumberOfValidCategories:
                    LOG_ERROR_S << "Internal Error: Got invalid Category";
                    return;
            }
        }
    }
    else
    {
        LOG_WARN_S << "got unsupported type..";
        return;
    }
    
    if (valueS != oldValue)
    {
        if (codec)
        {
            QTextCodec::ConverterState state;
            const QString text = codec->toUnicode(valueS.c_str(), valueS.size(), &state);
            
            if (state.invalidChars > 0)
            {
                return;
            }
            
            value->setText(text);
        }
    }
}

EditableSimple::EditableSimple(TypedItem *name, TypedItem *value)
    : Simple(name, value)
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

void EditableSimple::update(Typelib::Value& valueIn, bool updateUI)
{
    emit editableUpdate(valueIn);

    Simple::update(valueIn, updateUI);
}

template <class T>
bool setValue(Typelib::Value& value, std::string const &valueS)
{
    T *val = static_cast<T *>(value.getData());

    if (!val)
    {
        return false;
    }

    try
    {
        T newval = boost::lexical_cast<T>(valueS);
        if (*val == newval)
            return false;
        *val = newval;
        return true;
    }
    catch (...)
    {

    }

    return false;
}

template <>
bool setValue<uint8_t>(Typelib::Value& value, std::string const &valueS)
{
    uint8_t *val = static_cast<uint8_t *>(value.getData());

    if (!val)
    {
        return false;
    }

    try
    {
        int newval = boost::lexical_cast<int>(valueS);
        if (*val == newval)
            return false;
        *val = newval;
        return true;
    }
    catch (...)
    {

    }

    return false;
}

template <>
bool setValue<int8_t>(Typelib::Value& value, std::string const &valueS)
{
    int8_t *val = static_cast<int8_t *>(value.getData());

    if (!val)
    {
        return false;
    }

    try
    {
        int newval = boost::lexical_cast<int>(valueS);
        if (*val == newval)
            return false;
        *val = newval;
        return true;
    }
    catch (...)
    {

    }

    return false;
}

bool EditableSimple::updateFromEdit()
{
    for(auto &h : handlerStack)
    {
        if (h->flags() & ConfigItemHandler::Flags::ConvertsToTypelibValue)
        {
            return h->convertToTypelibValue(value_handle, value, codec);
        }
    }

    QString data = value->data(Qt::EditRole).toString();

    const Typelib::Type &type(value_handle.getType());
    std::string valueS = data.toStdString();
    if (codec)
    {
        QByteArray bytes = codec->fromUnicode(data);
        valueS = bytes.toStdString();
    }

    if (type.getCategory() == Typelib::Type::Enum)
    {
        const Typelib::Enum &enumT = static_cast<const Typelib::Enum &>(value_handle.getType());
        Typelib::Enum::integral_type *intVal = (static_cast<Typelib::Enum::integral_type *>(value_handle.getData()));
        if (!intVal)
            return false;
        if (*intVal == enumT.get(valueS))
            return false;
        *intVal = enumT.get(valueS);
        return true;
    }
    else if (type.getCategory() == Typelib::Type::Numeric)
    {
        const Typelib::Numeric &numeric(static_cast<const Typelib::Numeric &>(value_handle.getType()));
        if (numeric.getName() == "/bool")
        {
            bool *boolVal = static_cast<bool *>(value_handle.getData());
            if (!boolVal)
                return false;
            bool b = data.compare("true",Qt::CaseInsensitive) == 0;
            if (*boolVal == b)
                return false;
            *boolVal = b;
            return true;
        }
        else
        {
            switch(numeric.getNumericCategory())
            {
                case Typelib::Numeric::Float:
                    if(numeric.getSize() == sizeof(float))
                    {
                        return setValue<float>(value_handle, valueS);
                    }
                    else
                    {
                        return setValue<double>(value_handle, valueS);
                    }
                    break;
                case Typelib::Numeric::SInt:
                    switch(numeric.getSize())
                    {
                        case sizeof(int8_t):
                            return setValue<int8_t>(value_handle, valueS);
                        case sizeof(int16_t):
                            return setValue<int16_t>(value_handle, valueS);
                        case sizeof(int32_t):
                            return setValue<int32_t>(value_handle, valueS);
                        case sizeof(int64_t):
                            return setValue<int64_t>(value_handle, valueS);
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
                            return setValue<uint8_t>(value_handle, valueS);
                        case sizeof(uint16_t):
                            return setValue<uint16_t>(value_handle, valueS);
                        case sizeof(uint32_t):
                            return setValue<uint32_t>(value_handle, valueS);
                        case sizeof(uint64_t):
                            return setValue<uint64_t>(value_handle, valueS);
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

    return false;
}

bool EditableSimple::compareAndMark ( Typelib::Value& valueCurrent, Typelib::Value& valueOld )
{
    bool isEqual = Typelib::compare(valueCurrent, valueOld);

    name->QStandardItem::setData(QVariant(!isEqual), ItemBase::ModifiedRole);
    value->QStandardItem::setData(QVariant(!isEqual), ItemBase::ModifiedRole);

    return isEqual;
}

Complex::Complex(TypedItem *name, TypedItem *value)
    : ItemBase(name, value)
{
}

Complex::~Complex()
{

}

std::shared_ptr<ItemBase> Complex::getItem(Typelib::Value& value, TypedItem *nameItem, TypedItem *valueItem) const
{
    return ::getItem(value, handlerrepo, nameItem, valueItem);
}

void Complex::update(Typelib::Value& valueIn, bool updateUI)
{       
    bool childrenUpdateUI = updateUI && name->isExpanded();
    const Typelib::Type &type(valueIn.getType());

    value_handle = valueIn;

    emit visualizerUpdate(valueIn);

    bool haveCustomValue = false;

    for(auto &h : handlerStack)
    {
        if (h->flags() & ConfigItemHandler::Flags::ConvertsFromTypelibValue)
        {
            if (updateUI)
            {
                h->convertFromTypelibValue(value, valueIn, codec);
            }
            haveCustomValue = true;
        }
        if (h->flags() & ConfigItemHandler::Flags::HideFields)
        {
            return;
        }
    }

    if (!haveCustomValue && updateUI)
    {
        this->value->setText(valueIn.getType().getName().c_str());
    }

    if (type.getCategory() == Typelib::Type::Compound)
    {
        const Typelib::Compound &comp = static_cast<const Typelib::Compound &>(type);
        uint8_t *data = static_cast<uint8_t *>(valueIn.getData());
        size_t i = 0;
        for (const Typelib::Field &field: comp.getFields())
        {
            Typelib::Value fieldV(data + field.getOffset(), field.getType());
            
            if (children.size() <= i)
            {
                if(!updateUI)
                {
                    break;
                }
                std::shared_ptr<ItemBase> newVal = getItem(fieldV);
                newVal->setName(field.getName().c_str());
                children.push_back(newVal);
                name->appendRow(newVal->getRow());
            }

            children[i]->update(fieldV, childrenUpdateUI);
            i++;
        }
    }
    else
    {
        const Typelib::Container &cont = static_cast<const Typelib::Container &>(valueIn.getType());
        const size_t size = cont.getElementCount(valueIn.getData());

        //std::vector
        int numElemsToDisplay = static_cast<int>(size);
        bool areMaxElemsDiplayed = false;
        // limit max elems displayed to maxVectorElemsDisplayed
        if (numElemsToDisplay > maxVectorElemsDisplayed)
        {
            numElemsToDisplay = maxVectorElemsDisplayed;
            areMaxElemsDiplayed = true;
        }
        
        int currentRows = updateUI?name->rowCount():children.size();
        
        // check if number of displayed items changed
        bool numElemsDisplayedChanged = (numElemsToDisplay != currentRows);
        
        if (currentRows > numElemsToDisplay && updateUI)
        {
            while (name->rowCount() > numElemsToDisplay)
            {   
                name->takeRow(name->rowCount() -1);
            }
            
            currentRows = numElemsToDisplay;
        }
        
        // update old (displayed) items
        // children size at this step is min(currentRows, numElemsToDisplay)
        for(int i = 0; i < std::min(currentRows, numElemsToDisplay); i++)
        {
            Typelib::Value elem = cont.getElement(valueIn.getData(), i);
            children[i]->update(elem, childrenUpdateUI);
        }
        
        if (updateUI)
        {
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

                child->update(elem, true);
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
        }
    }
}

EditableComplex::EditableComplex(TypedItem *name, TypedItem *value)
    : Complex(name, value)
{
    if(!name || !value)
    {
        this->name->setType(ItemType::EDITABLEITEM);
        this->value->setType(ItemType::EDITABLEITEM);
    }
}

void EditableComplex::setHandlerStack(std::vector<ConfigItemHandler const*> const &stack)
{
    Complex::setHandlerStack(stack);
    for(auto &h : handlerStack)
    {
        if (h->flags() & (ConfigItemHandler::Flags::ConvertsToTypelibValue |
                ConfigItemHandler::Flags::CustomEditor))
        {
            if(value)
                value->setEditable(true);
            return;
        }
    }
}

EditableComplex::~EditableComplex()
{
}

Typelib::Value& EditableComplex::getValueHandle()
{
    return value_handle;
}

void EditableComplex::update(Typelib::Value& valueIn, bool updateUI)
{
    emit editableUpdate(valueIn);

    Complex::update(valueIn, updateUI);
}

std::shared_ptr<ItemBase> EditableComplex::getItem(Typelib::Value& value, TypedItem *nameItem, TypedItem *valueItem) const
{
    return ::getEditableItem(value, handlerrepo, nameItem, valueItem);
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
            itembase = std::shared_ptr<ItemBase>(new Complex(nameItem, valueItem));
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
    itembase->setHandlerStack(handlerrepo->findNameServiceConfigItemHandlersFor(type, false));
    return itembase;
}

std::shared_ptr< ItemBase > getEditableItem(Typelib::Value& value, ConfigItemHandlerRepository *handlerrepo, TypedItem *nameItem, TypedItem *valueItem)
{
    const Typelib::Type &type(value.getType());

    std::shared_ptr<ItemBase> itembase;

    switch(type.getCategory())
    {
        case Typelib::Type::Array:
            itembase = std::shared_ptr<ItemBase>(new EditableArray(nameItem, valueItem));
            break;
        case Typelib::Type::Enum:
        case Typelib::Type::Numeric:
            itembase = std::shared_ptr<ItemBase>(new EditableSimple(nameItem, valueItem));
            break;
        case Typelib::Type::Compound:
        case Typelib::Type::Container:
            itembase = std::shared_ptr<ItemBase>(new EditableComplex(nameItem, valueItem));
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
    itembase->setHandlerStack(handlerrepo->findNameServiceConfigItemHandlersFor(type, true));
    return itembase;
}

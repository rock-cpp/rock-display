
/* including this first because it uses "emit" and "signals" which are
 * #defined by QT
 *
 * workaround that works in some compilers:
 * #pragma push_macro("emit")
 * #pragma push_macro("signals")
 * #undef emit
 * #undef signals
 */
#include <rtt/transports/corba/TaskContextProxy.hpp>
/*
 * #pragma pop_macro("signals")
 * #pragma pop_macro("emit")
 */

#include "vizkitplugin_p.hpp"
#include <rtt/base/OutputPortInterface.hpp>
#include <rtt/base/InputPortInterface.hpp>
#include <rtt/base/PropertyBase.hpp>
#include <rtt/types/TypeInfo.hpp>
#include "../TypedItem.hpp"
#include "../Types.hpp"
#include "../ConfigItem.hpp"
#include "../PortItem.hpp"
#include "../PropertyItem.hpp"
#include <iterator>
#include <sstream>

using namespace rockdisplay;

FieldDescriptionImpl::FieldDescriptionImpl
(RTT::base::OutputPortInterface *outputport,
 RTT::corba::TaskContextProxy *taskContextProxy,
 orocos_cpp::NameService *nameService,
 Typelib::Type const *type, Typelib::Registry const *registry,
 std::string const &fieldName, TypedItem *ti, QObject *parent)
    : QObject(parent), outputport(outputport), inputport(nullptr),
      property(nullptr), taskContextProxy(taskContextProxy),
      nameService(nameService), type(type), registry(registry),
      fieldName(fieldName), ti(ti)
{
}

FieldDescriptionImpl::FieldDescriptionImpl
(RTT::base::InputPortInterface *inputport,
 RTT::corba::TaskContextProxy *taskContextProxy,
 orocos_cpp::NameService *nameService,
 Typelib::Type const *type, Typelib::Registry const *registry,
 std::string const &fieldName, TypedItem *ti, QObject *parent)
    : QObject(parent), outputport(nullptr), inputport(inputport),
      property(nullptr), taskContextProxy(taskContextProxy),
      nameService(nameService), type(type), registry(registry),
      fieldName(fieldName), ti(ti)
{
}

FieldDescriptionImpl::FieldDescriptionImpl
(RTT::base::PropertyBase *property,
 RTT::corba::TaskContextProxy *taskContextProxy,
 orocos_cpp::NameService *nameService,
 Typelib::Type const *type, Typelib::Registry const *registry,
 std::string const &fieldName, TypedItem *ti, QObject *parent)
    : QObject(parent), outputport(nullptr), inputport(nullptr),
      property(property), taskContextProxy(taskContextProxy),
      nameService(nameService), type(type), registry(registry),
      fieldName(fieldName), ti(ti)
{
}

FieldDescriptionImpl::~FieldDescriptionImpl()
{
}

void FieldDescriptionImpl::setType(Typelib::Type const *type) {
    this->type = type;
}

RTT::base::OutputPortInterface *FieldDescriptionImpl::getOutputPort() const
{
    return outputport;
}

RTT::base::InputPortInterface *FieldDescriptionImpl::getInputPort() const
{
    return inputport;
}

RTT::base::PropertyBase *FieldDescriptionImpl::getProperty() const
{
    return property;
}

RTT::corba::TaskContextProxy *FieldDescriptionImpl::getTaskContextProxy() const {
    return taskContextProxy;
}

orocos_cpp::NameService *FieldDescriptionImpl::getNameService() const {
    return nameService;
}

Typelib::Type const *FieldDescriptionImpl::getType() const
{
    return type;
}

Typelib::Registry const *FieldDescriptionImpl::getRegistry() const
{
    return registry;
}

std::string FieldDescriptionImpl::getTaskName() const
{
    return taskContextProxy->getName();
}

std::string FieldDescriptionImpl::getOutputPortName() const
{
    if (outputport)
    {
        return outputport->getName();
    }
    else
    {
        return std::string();
    }
}

std::string FieldDescriptionImpl::getInputPortName() const
{
    if (inputport)
    {
        return inputport->getName();
    }
    else
    {
        return std::string();
    }
}

std::string FieldDescriptionImpl::getPropertyName() const
{
    if (property)
    {
        return property->getName();
    }
    else
    {
        return std::string();
    }
}

std::string FieldDescriptionImpl::getFieldName() const
{
    return fieldName;
}

std::string FieldDescriptionImpl::getTypeName() const
{
    if(type)
        return type->getName();
    else
        return outputport->getTypeInfo()->getTypeName();
}

vizkitplugin::ValueConverter const *FieldDescriptionImpl::constructValueConverter() const
{
    return ValueConverterImpl::fromTypedItem(ti);
}

XlatingFieldDescriptionImpl::XlatingFieldDescriptionImpl
(RTT::base::OutputPortInterface *outputport,
 RTT::corba::TaskContextProxy *taskContextProxy,
 orocos_cpp::NameService *nameService,
 Typelib::Type const *type, Typelib::Registry const *registry,
 std::string const &fieldName, TypedItem *ti,
 TypeConverterFactory *converterfactory, QObject *parent)
    : FieldDescriptionImpl(outputport, taskContextProxy, nameService,
                                type, registry, fieldName, ti, parent),
      converterfactory(converterfactory)
{
}

XlatingFieldDescriptionImpl::XlatingFieldDescriptionImpl
(RTT::base::InputPortInterface *inputport,
 RTT::corba::TaskContextProxy *taskContextProxy,
 orocos_cpp::NameService *nameService,
 Typelib::Type const *type, Typelib::Registry const *registry,
 std::string const &fieldName, TypedItem *ti,
 TypeConverterFactory *converterfactory, QObject *parent)
    : FieldDescriptionImpl(inputport, taskContextProxy, nameService,
                                type, registry, fieldName, ti, parent),
      converterfactory(converterfactory)
{
}

XlatingFieldDescriptionImpl::XlatingFieldDescriptionImpl
(RTT::base::PropertyBase *property,
 RTT::corba::TaskContextProxy *taskContextProxy,
 orocos_cpp::NameService *nameService,
 Typelib::Type const *type, Typelib::Registry const *registry,
 std::string const &fieldName, TypedItem *ti,
 TypeConverterFactory *converterfactory, QObject *parent)
    : FieldDescriptionImpl(property, taskContextProxy, nameService,
                                type, registry, fieldName, ti, parent),
      converterfactory(converterfactory)
{
}

Typelib::Type const *XlatingFieldDescriptionImpl::getType() const
{
    return getRegistry()->get(converterfactory->getResultTypename());
}

std::string XlatingFieldDescriptionImpl::getTypeName() const
{
    return converterfactory->getResultTypename();
}

vizkitplugin::ValueConverter const *XlatingFieldDescriptionImpl::constructValueConverter() const
{
    return nullptr;//This is not possible to do without c++ level introspection.
    //1) Starting from the typelib structure that can be marshalled to an arbitrary subfield of the
    //   c++ type, one would need to first convert to the c++ type and then somehow find the
    //   subfield through the actual maps, vectors and other pointer laden types.
    //2) the API cannot return results that are not already in memory.
    //3) the API would need to be notified whenever a back-conversion is needed where currently
    //   only the property/port need to know about it.
}


ValueHandleImpl::ValueHandleImpl(Typelib::Value const &value,
                                 vizkitplugin::FieldDescription *fieldhandle, QObject *parent)
    : QObject(parent), value(value), fieldhandle(fieldhandle)
{
}

ValueHandleImpl::~ValueHandleImpl()
{
}

void ValueHandleImpl::edited(bool forceSend)
{
    emit editedSignal(forceSend);
}

vizkitplugin::FieldDescription *ValueHandleImpl::getFieldDescription() const
{
    return fieldhandle;
}

Typelib::Value ValueHandleImpl::getValue() const
{
    return value;
}

void const *ValueHandleImpl::getRawPtr() const
{
    return rawptr;
}

void *ValueHandleImpl::getRawPtr()
{
    return rawptr;
}


void ValueHandleImpl::setValue(Typelib::Value const &value, void *rawptr)
{
    this->value = value;
    this->rawptr = rawptr;
}


ValueConverterImpl::ValueConverterImpl()
{
}

ValueConverterImpl::~ValueConverterImpl()
{
}

bool ValueConverterImpl::buildFromTypedItem(ItemBase*item, TypedItem *ti,TypedItem *pti,Typelib::Type const *&ptype_result, std::stringstream &fieldNameStream)
{
    Typelib::Type const *ptype = nullptr;
    ItemBase *pitem;
    switch (pti->type())
    {
        case OUTPUTPORT:
            pitem = static_cast<PortItem *>(pti->getData())->getItemBase().get();
            ptype = &(static_cast<PortItem *>(pti->getData())->getValueHandle().getType());
            baseType = ptype;
            break;
        case INPUTPORT:
            pitem = static_cast<PortItem *>(pti->getData())->getItemBase().get();
            ptype = &(static_cast<PortItem *>(pti->getData())->getValueHandle().getType());
            baseType = ptype;
            break;
        case PROPERTYITEM:
            pitem = static_cast<PropertyItem *>(pti->getData())->getItemBase().get();
            ptype = &(static_cast<PropertyItem *>(pti->getData())->getValueHandle().getType());
            baseType = ptype;
            break;
        case CONFIGITEM:
        case EDITABLEITEM:
        {
            pitem = static_cast<ItemBase *>(pti->getData());
            TypedItem *ppti = dynamic_cast<TypedItem *>(pti->parent());
            if (!ppti)
            {
                return false;
            }
            if(!buildFromTypedItem(pitem,pti,ppti,ptype,fieldNameStream))
            {
                return false;
            }
            break;
        }
        default:
            return false;
    }

    ssize_t index = pitem->indexOf(item);
    if (index == -1)
        return false;
    if (!ptype)
        return false;
    switch (ptype->getCategory())
    {
        case Typelib::Type::Category::Array:
        {
            Typelib::Array const *a = static_cast<Typelib::Array const *>(ptype);
            locator.skipBytes(a->getIndirection().getSize() * index);
            ptype_result = &a->getIndirection();
            fieldNameStream << "[" << index << "]";
            break;
        }
        case Typelib::Type::Category::Container:
        {
            Typelib::Container const *c = static_cast<Typelib::Container const *>(ptype);
            locator.addContainer(*c, index);
            ptype_result = &c->getIndirection();
            fieldNameStream << "[" << index << "]";
            break;
        }
        case Typelib::Type::Category::Compound:
        {
            Typelib::Compound const *c = static_cast<Typelib::Compound const *>(ptype);
            auto fl = c->getFields();
            auto f = std::next(fl.begin(), index);
            if (f == fl.end())
                return false;
            fieldNameStream << "." << f->getName();
            locator.skipBytes(f->getOffset());
            ptype_result = &f->getType();
            break;
        }
        default:
            //the rest is all single values, at which point we are done.
            break;
    }
    return true;
}

ValueConverterImpl *ValueConverterImpl::fromTypedItem(TypedItem *ti)
{
    ValueConverterImpl *impl = new ValueConverterImpl();
    Typelib::Type const *ptype;
    std::stringstream fieldNameStream;
    switch (ti->type())
    {
        case OUTPUTPORT:
            ptype = &(static_cast<PortItem *>(ti->getData())->getValueHandle().getType());
            impl->baseType = ptype;
            impl->fieldType = ptype;
            //we are done, this is the trivial conversion.
            return impl;
        case INPUTPORT:
            ptype = &(static_cast<PortItem *>(ti->getData())->getValueHandle().getType());
            impl->baseType = ptype;
            impl->fieldType = ptype;
            //we are done, this is the trivial conversion.
            return impl;
        case PROPERTYITEM:
            ptype = &(static_cast<PropertyItem *>(ti->getData())->getValueHandle().getType());
            impl->baseType = ptype;
            impl->fieldType = ptype;
            //we are done, this is the trivial conversion.
            return impl;
        case CONFIGITEM:
        case EDITABLEITEM:
        {
            //build instructions on how to find this item in its parents
            ItemBase *item = static_cast<ItemBase *>(ti->getData());
            //fill in array(s) up to here.
            TypedItem *pti = dynamic_cast<TypedItem *>(ti->parent());
            if (!pti)
            {
                return nullptr;
            }
            if (!impl->buildFromTypedItem(item,ti,pti,ptype,fieldNameStream))
            {
                delete (impl);
                return nullptr;
            }
            return impl;
            break;
        }
        default:
            return nullptr;
    }
    impl->fieldType = ptype;
    impl->fieldName = fieldNameStream.str();
    return impl;
}

Typelib::Value ValueConverterImpl::fieldValueFromBaseValue(Typelib::Value &basevalue) const
{
    uint8_t *pos = reinterpret_cast<uint8_t *>(basevalue.getData());
    for (auto &l : locator.skips) {
        pos += l.offset_bytes;
        if(l.container) {
            size_t count = l.container->getElementCount(pos);
            if(l.container_index >= count)
                return Typelib::Value();
            pos = reinterpret_cast<uint8_t *>
                  (l.container->getElement(pos, l.container_index).getData());
        }
    }
    return Typelib::Value(pos, *fieldType);
}

void * ValueConverterImpl::rawFieldValueFromBaseValue(Typelib::Value &basevalue) const {
    return fieldValueFromBaseValue(basevalue).getData();
}

void ValueConverterImpl::rawFieldValueUpdated(void *fieldValue, Typelib::Value &basevalue) const {
    //does nothing, the data changes automatically.
}

Typelib::Type const *ValueConverterImpl::getBaseType() const
{
    return baseType;
}

Typelib::Type const *ValueConverterImpl::getFieldType() const
{
    return fieldType;
}

std::string ValueConverterImpl::getFieldName() const
{
    return fieldName;
}

std::string ValueConverterImpl::getBaseTypeName() const
{
    return baseType->getName();
}

std::string ValueConverterImpl::getFieldTypeName() const
{
    return fieldType->getName();
}

//TODO create ValueConverterImpl for Aliasing types (i.E. the same as ValueConverterImpl, just different  result for getFieldTypeName
//TODO create ValueConverterImpl for Types getting converted by the TypelibMarshaller. Those
// would not provide fieldValueFromBaseValue, but use the ValueConverterImpl::fieldValueFromBaseValue
//  as basis for the value converted by the TypelibMarshaller which will then be returned by
//  rawFieldValueFromBaseValue. rawFieldValueUpdated will do the inverse conversion into the
//  value from ValueConverterImpl::fieldValueFromBaseValue


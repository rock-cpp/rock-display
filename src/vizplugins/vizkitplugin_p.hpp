
#pragma once

#include "vizkitplugin.hpp"
#include <typelib/value.hh>
#include <typelib/memory_layout.hh>
#include <typelib/typemodel.hh>
#include "../TypeConverterFactory.hpp"
#include <memory>

class TypedItem;
class ItemBase;

namespace rockdisplay {

class FieldDescriptionImpl : public QObject, public vizkitplugin::FieldDescription
{
    Q_OBJECT;
private:
    RTT::base::OutputPortInterface *outputport;
    RTT::base::InputPortInterface *inputport;
    RTT::base::PropertyBase *property;
    RTT::corba::TaskContextProxy *taskContextProxy;
    orocos_cpp::NameService *nameService;
    Typelib::Type const *type;
    Typelib::Registry const *registry;
    std::string fieldName;
    TypedItem* ti;
public:
    FieldDescriptionImpl(RTT::base::OutputPortInterface *outputport,
                    RTT::corba::TaskContextProxy *taskContextProxy,
                    orocos_cpp::NameService *nameService,
                    Typelib::Type const *type,
                    Typelib::Registry const *registry,
                    std::string const &fieldName,
                    TypedItem *ti, QObject *parent = nullptr);
    FieldDescriptionImpl(RTT::base::InputPortInterface *inputport,
                    RTT::corba::TaskContextProxy *taskContextProxy,
                    orocos_cpp::NameService *nameService,
                    Typelib::Type const *type,
                    Typelib::Registry const *registry,
                    std::string const &fieldName,
                    TypedItem *ti, QObject *parent = nullptr);
    FieldDescriptionImpl(RTT::base::PropertyBase *property,
                    RTT::corba::TaskContextProxy *taskContextProxy,
                    orocos_cpp::NameService *nameService,
                    Typelib::Type const *type,
                    Typelib::Registry const *registry,
                    std::string const &fieldName,
                    TypedItem *ti, QObject *parent = nullptr);
    virtual ~FieldDescriptionImpl() override;
    virtual void setType(Typelib::Type const *type);
    virtual RTT::base::OutputPortInterface *getOutputPort() const override;
    virtual RTT::base::InputPortInterface *getInputPort() const override;
    virtual RTT::base::PropertyBase *getProperty() const override;
    virtual RTT::corba::TaskContextProxy *getTaskContextProxy() const override;
    virtual orocos_cpp::NameService* getNameService() const override;
    virtual Typelib::Type const *getType() const override;
    virtual const Typelib::Registry* getRegistry() const override;
    virtual std::string getTaskName() const override;
    virtual std::string getOutputPortName() const override;
    virtual std::string getInputPortName() const override;
    virtual std::string getPropertyName() const override;
    virtual std::string getFieldName() const override;
    virtual std::string getTypeName() const override;
    virtual vizkitplugin::ValueConverter const *constructValueConverter() const override;
};

class XlatingFieldDescriptionImpl : public FieldDescriptionImpl
{
    Q_OBJECT;
private:
    TypeConverterFactory *converterfactory;
public:
    XlatingFieldDescriptionImpl(RTT::base::OutputPortInterface *
                                     outputport,
                                     RTT::corba::TaskContextProxy *
                                     taskContextProxy,
                                     orocos_cpp::NameService *nameService,
                                     Typelib::Type const *type,
                                     Typelib::Registry const *registry,
                                     std::string const &fieldName,
                                     TypedItem *ti,
                                     TypeConverterFactory *converterfactory,
                                     QObject *parent = nullptr);
    XlatingFieldDescriptionImpl(RTT::base::InputPortInterface *
                                     inputport,
                                     RTT::corba::TaskContextProxy *
                                     taskContextProxy,
                                     orocos_cpp::NameService *nameService,
                                     Typelib::Type const *type,
                                     Typelib::Registry const *registry,
                                     std::string const &fieldName,
                                     TypedItem *ti,
                                     TypeConverterFactory *converterfactory,
                                     QObject *parent = nullptr);
    XlatingFieldDescriptionImpl(RTT::base::PropertyBase *
                                     property,
                                     RTT::corba::TaskContextProxy *
                                     taskContextProxy,
                                     orocos_cpp::NameService *nameService,
                                     Typelib::Type const *type,
                                     Typelib::Registry const *registry,
                                     std::string const &fieldName,
                                     TypedItem *ti,
                                     TypeConverterFactory *converterfactory,
                                     QObject *parent = nullptr);
    virtual Typelib::Type const *getType() const override;
    virtual std::string getTypeName() const override;
    virtual vizkitplugin::ValueConverter const *constructValueConverter() const
    override;
};

class ValueHandleImpl : public QObject, public vizkitplugin::ValueHandle
{
    Q_OBJECT;
private:
    Typelib::Value value;
    vizkitplugin::FieldDescription *fieldhandle;
    void *rawptr;
public:
    ValueHandleImpl(Typelib::Value const &value, vizkitplugin::FieldDescription *fieldhandle, QObject *parent = nullptr);
    virtual ~ValueHandleImpl() override;
    virtual void edited(bool forceSend) override;
    virtual vizkitplugin::FieldDescription *getFieldDescription() const override;
    virtual Typelib::Value getValue() const override;
    virtual void const *getRawPtr() const override;
    virtual void *getRawPtr() override;

    void setValue(Typelib::Value const &value, void *rawptr);
signals:
    void editedSignal(bool forceSend);
};

class ValueConverterImpl : public vizkitplugin::ValueConverter {
private:
    class ElementLocator
    {
    public:
        class OffsetInfo
        {
        public:
            size_t offset_bytes;
            Typelib::Container const *container; //may be nullptr for last element, if unused
            size_t container_index; //may be ~0ULL for last element, if unused
            OffsetInfo() : offset_bytes(0), container(nullptr), container_index(~0ULL) {}
        };
        std::vector<OffsetInfo> skips;
        void skipBytes(size_t num)
        {
            if (skips.empty() || skips.back().container)
            {
                skips.push_back(OffsetInfo());
            }
            skips.back().offset_bytes += num;
        }
        void addContainer(Typelib::Container const &c, size_t index)
        {
            if (skips.empty() || skips.back().container)
            {
                skips.push_back(OffsetInfo());
            }
            skips.back().container = &c;
            skips.back().container_index = index;
        }
    };
    ElementLocator locator;
    Typelib::Type const *fieldType, *baseType;
    std::string fieldName;
    ValueConverterImpl();
    bool buildFromTypedItem(ItemBase *item, TypedItem *ti,TypedItem *pti,Typelib::Type const *&ptype_result, std::stringstream &fieldNameStream);
public:
    static ValueConverterImpl *fromTypedItem(TypedItem *ti);
    virtual ~ValueConverterImpl() override;
    virtual Typelib::Value fieldValueFromBaseValue(Typelib::Value &basevalue) const override;
    virtual void * rawFieldValueFromBaseValue(Typelib::Value &basevalue) const override;
    virtual void rawFieldValueUpdated(void *fieldValue, Typelib::Value &basevalue) const override;
    virtual Typelib::Type const *getBaseType() const override;
    virtual Typelib::Type const *getFieldType() const override;
    virtual std::string getFieldName() const override;
    virtual std::string getBaseTypeName() const override;
    virtual std::string getFieldTypeName() const override;
};

}

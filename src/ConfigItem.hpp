#pragma once

#include "TypedItem.hpp"
#include "VisualizerAdapter.hpp"
#include <typelib/value.hh>
#include <rtt/base/DataSourceBase.hpp>

QT_BEGIN_NAMESPACE
class QTextCodec;
QT_END_NAMESPACE

class PortHandle;
class OutputPortItem;
class VizHandle;
class PortItem;
class ConfigItemHandlerRepository;

namespace RTT
{
    namespace types
    {
        class TypeInfo;
    }
    
    namespace base
    {
        class OutputPortInterface;
        class InputPortInterface;
        class PortInterface;
    }
}

class ConfigItemHandler;

class ItemBase : public VisualizerAdapter
{
protected:
    TypedItem *name;
    TypedItem *value;
    
    std::vector<std::shared_ptr<ItemBase> > children;
    ConfigItemHandlerRepository *handlerrepo;
    
    /** Stack of handlers associated with this item.
     *
     * Each of these handlers may decide to produce a custom textual output,
     * a custom editor, custom context menu entries or similar. It may also
     * decide it fully handled this item and there is no more to do(TODO: do we want to do this?).
     *
     * The handlerStack is determined during getItem and getEditableItem
     */
    std::vector<ConfigItemHandler const*> handlerStack;
public:
    enum UsedRoles {
        ModifiedRole = Qt::UserRole + 1
    };


    ItemBase();
    ItemBase(TypedItem *name, TypedItem *value);
    virtual ~ItemBase();
    
    QTextCodec *codec;
    static std::map<std::string, std::string> marshalled2Typelib;
    
    virtual bool hasVisualizers();
    
    /*
     * @param valueIn     the value to be handled by the item
     * @param base_sample A reference to the full Sample that valueIn is part of.
     *                    This can be used to keep the sample and thus the data in valueIn alive.
     * @param updateUI    ?
     * @param forceUpdate ?
     * @return            ?
     */
    virtual bool update(Typelib::Value& valueIn, RTT::base::DataSourceBase::shared_ptr base_sample, bool updateUI = true, bool forceUpdate = false) = 0;
    virtual Typelib::Value& getValueHandle();
    virtual RTT::base::DataSourceBase::shared_ptr getBaseSample() { return nullptr; }
    virtual bool compareAndMark(Typelib::Value& valueCurrent, Typelib::Value& valueOld) { return false; }

    void setName(const QString &newName)
    {
        name->setText(newName);
    }
    
    QList<QStandardItem *> getRow()
    {
        return {name, value};
    }
    
    void setType(int newType)
    {
        this->name->setType(newType);
        this->value->setType(newType);
    }
    
    std::vector<std::shared_ptr<ItemBase> > const &getChildren() const
    {
        return this->children;
    }
    
    TypedItem *getName()
    {
        return this->name;
    }

    void setHandlerRepo(ConfigItemHandlerRepository *handlerrepo)
    {
        this->handlerrepo = handlerrepo;
    }

    virtual void setHandlerStack(std::vector<ConfigItemHandler const*> const &stack)
    {
        handlerStack = stack;
    }

    std::vector<ConfigItemHandler const*> const &getHandlerStack()
    {
        return handlerStack;
    }

    static std::map<std::string, std::string> lookupMarshalledTypelistTypes();
};

/** Returns a static ItemBase config item
 *
 * @param value      Used to populate initial values.
 * @param nameItem   TypedItem to use for name column. Will be generated if nullptr.
 * @param valueItem  TypedItem to use for value column. Will be generated if nullptr.
 * @return  One of Array, Simple, Complex.
 */
std::shared_ptr<ItemBase> getItem(Typelib::Value& value, ConfigItemHandlerRepository *handlerrepo, TypedItem *nameItem = nullptr, TypedItem *valueItem = nullptr);

/** Returns an editable ItemBase config item
 *
 * @param value      Used to store changed data. must be kept alive for the lifetime of the item.
 * @param nameItem   TypedItem to use for name column. Will be generated if nullptr.
 * @param valueItem  TypedItem to use for value column. Will be generated if nullptr.
 * @return one of EditableArray, EditableSimple, EditableComplex.
 */
std::shared_ptr<ItemBase> getEditableItem(Typelib::Value& value, ConfigItemHandlerRepository *handlerrepo, TypedItem *nameItem = nullptr, TypedItem *valueItem = nullptr);

class Array : public ItemBase
{   
protected:
    const int maxArrayElemsDisplayed = 500;

    virtual std::shared_ptr<ItemBase> getItem(Typelib::Value& value, TypedItem *nameItem = nullptr, TypedItem *valueItem = nullptr) const;

public:
    Array(TypedItem *name = nullptr, TypedItem *value = nullptr);
    virtual ~Array();
    
    virtual bool update(Typelib::Value& valueIn, RTT::base::DataSourceBase::shared_ptr base_sample, bool updateUI = false, bool forceUpdate = false);
};

class EditableArray : public Array
{
private:
    Typelib::Value value_handle;
    RTT::base::DataSourceBase::shared_ptr base_sample;
protected:
    virtual std::shared_ptr<ItemBase> getItem(Typelib::Value& value, TypedItem *nameItem = nullptr, TypedItem *valueItem = nullptr) const override;
public:
    EditableArray(Typelib::Value& valueIn, TypedItem *name = nullptr, TypedItem *value = nullptr);
    virtual ~EditableArray();
    virtual Typelib::Value& getValueHandle() override;
    virtual RTT::base::DataSourceBase::shared_ptr getBaseSample() override;
    virtual bool update(Typelib::Value& valueIn, RTT::base::DataSourceBase::shared_ptr base_sample, bool updateUI = false, bool forceUpdate = false) override;
    virtual bool compareAndMark(Typelib::Value& valueCurrent, Typelib::Value& valueOld) override;
};

class Simple : public ItemBase
{
public:
    Simple(TypedItem *name = nullptr, TypedItem *value = nullptr);
    virtual ~Simple();
    
    virtual bool update(Typelib::Value& valueIn, RTT::base::DataSourceBase::shared_ptr base_sample, bool updateUI = false, bool forceUpdate = false);
};

class EditableSimple : public Simple
{
private:
    Typelib::Value value_handle;
    RTT::base::DataSourceBase::shared_ptr base_sample;
public:
    EditableSimple(Typelib::Value& valueIn, TypedItem *name = nullptr, TypedItem *value = nullptr);
    virtual ~EditableSimple();
    virtual Typelib::Value& getValueHandle() override;
    virtual RTT::base::DataSourceBase::shared_ptr getBaseSample() override;
    virtual bool update(Typelib::Value& valueIn, RTT::base::DataSourceBase::shared_ptr base_sample, bool updateUI = false, bool forceUpdate = false) override;
    bool updateFromEdit();
    virtual bool compareAndMark(Typelib::Value& valueCurrent, Typelib::Value& valueOld) override;
};

class Complex : public ItemBase
{
protected:
    const int maxVectorElemsDisplayed = 500;

    virtual std::shared_ptr<ItemBase> getItem(Typelib::Value& value, TypedItem *nameItem = nullptr, TypedItem *valueItem = nullptr) const;

public:
    Complex(TypedItem *name = nullptr, TypedItem *value = nullptr);
    virtual ~Complex();
    
    virtual bool update(Typelib::Value& valueIn, RTT::base::DataSourceBase::shared_ptr base_sample, bool updateUI = false, bool forceUpdate = false);
};

class EditableComplex : public Complex
{
private:
    Typelib::Value value_handle;
    RTT::base::DataSourceBase::shared_ptr base_sample;
protected:
    virtual std::shared_ptr<ItemBase> getItem(Typelib::Value& value, TypedItem *nameItem = nullptr, TypedItem *valueItem = nullptr) const override;
public:
    EditableComplex(Typelib::Value& valueIn, TypedItem *name = nullptr, TypedItem *value = nullptr);
    virtual ~EditableComplex();
    virtual Typelib::Value& getValueHandle() override;
    virtual RTT::base::DataSourceBase::shared_ptr getBaseSample() override;
    virtual bool update(Typelib::Value& valueIn, RTT::base::DataSourceBase::shared_ptr base_sample, bool updateUI = false, bool forceUpdate = false) override;
    virtual void setHandlerStack(std::vector<ConfigItemHandler const*> const &stack) override;
    virtual bool compareAndMark(Typelib::Value& valueCurrent, Typelib::Value& valueOld) override;
};

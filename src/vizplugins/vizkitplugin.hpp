
#pragma once

#include <QObject>
#include <mutex>
#include <boost/thread/shared_mutex.hpp>

namespace RTT {
namespace base {
class PropertyBase;
class OutputPortInterface;
class InputPortInterface;
}
namespace corba {
class TaskContextProxy;
}
}

namespace Typelib {
class Type;
class Value;
class Registry;
}

namespace orocos_cpp {
class NameService;
}

namespace rockdisplay {
namespace vizkitplugin {

class Plugin;
class Widget;
class Field;
class StandaloneSubplugin;
class FieldDescription;
class ValueHandle;

/** @defgroup vizkitpluginimpl Class interfaces to be implemented by plugins
 *  @{
 */

/**
 * @brief This interface allows creation of all Plugins
 *
 * This is the main entry point object that will be created to access
 * all Plugin objects from this plugin.
 */
class PluginFactory : public QObject {
    Q_OBJECT;

public:
    /**
     * @brief Called by rock-display to create all Plugin instances of this plugin
     *
     * @return A list of all Plugins instances
     */
    virtual std::vector<Plugin*> createPlugins() = 0;
};

/**
 * @brief Plugin interface
 *
 * This interface allows probing fields from OutputPorts, InputPorts and
 * Properties and allows creation of one or more Widget instances. The
 * fields are later added to a Widget.
 *
 * In addition, this provides a name, flags and a list of subplugins that
 * can be instantiated without any field.
 *
 * This is created by PluginFactory::createPlugins().
 */
class Plugin : public QObject {
    Q_OBJECT;
public:
    /**
     * @brief Flags that describe requirements/abilities of the Plugin
     *
     * These are 'or'ed together to form the return value of getFlags()
     */
    enum Flags {
        WidgetCanHandleMultipleFields = 2,//< The UI may offer to add ports to existing widgets. The plugin can still reject ports, e.g. when they don't fit with the rest.
        PreferSingleWidget = 4,//< The UI should not offer a choice between creating a new widget and adding to an existing one. Instead, it should only offer a choice when there are multiple widgets.
        SingleFieldOnly = 8,//< The UI should not offer to add more fields to a widget, instead should create new widgets. The plugin can reject adding ports in any case(even the first one).
        CanRemoveFields = 16,//< The UI offers removal of fields, in addition to removing the whole widget
        KeepOpenWithoutFields = 32,//< The UI will not automatically remove a widget without fields
        AllowDuplicateFields = 64,//< The UI will offer to readd a field even when it has already been added to the same widget for the same subplugin
        KeepWidgets = 128, //< The UI will keep any widgets returned by createWidget around forever and reuse as appropriate.
    };
    /**
     * @brief The constructor
     *
     * @param parent A parent for the QObject if the plugin wishes to use it
     *               for object lifetime management
     */
    Plugin(QObject *parent = nullptr) : QObject(parent) {}
    /**
     * @brief Check if a field of an OutputPort would be handled by this plugin
     *
     * The implementation would check against fieldDesc if it can handle this
     * field. The Typelib fields in fieldDesc may be unavailable when no data
     * has been seen yet.
     *
     * If the plugin supports subplugins, it has to fill subpluginnames with
     * the names that can be used to instantiate the subplugins with
     * Widget::addOutputPort.
     *
     * This is called _before_ a value is available to pupulate the context
     * menus plugin section.
     *
     * @param fieldDesc Description of the field
     * @param subpluginnames Array that is being filled with subplugin names
     *                       if supported. otherwise leave empty.
     * @return true if the field is handled in some way by this plugin
     */
    virtual bool probeOutputPort(FieldDescription *fieldDesc, std::vector<std::string> &subpluginnames) { return false; }
    /**
     * @brief Check if a field of an InputPort would be handled by this plugin
     *
     * The implementation would check against fieldDesc if it can handle this
     * field. The Typelib fields in fieldDesc may be unavailable when no data
     * has been seen yet.
     *
     * If the plugin supports subplugins, it has to fill subpluginnames with
     * the names that can be used to instantiate the subplugins with
     * Widget::addInputPort.
     *
     * This is called _before_ a value is available to pupulate the context
     * menus plugin section.
     *
     * @param fieldDesc Description of the field
     * @param subpluginnames Array that is being filled with subplugin names
     *                       if supported. otherwise leave empty.
     * @return true if the field is handled in some way by this plugin
     */
    virtual bool probeInputPort(FieldDescription *fieldDesc, std::vector<std::string> &subpluginnames) { return false; }
    /**
     * @brief Check if a field of a Property would be handled by this plugin
     *
     * The implementation would check against fieldDesc if it can handle this
     * field. The Typelib fields in fieldDesc may be unavailable when no data
     * has been seen yet.
     *
     * If the plugin supports subplugins, it has to fill subpluginnames with
     * the names that can be used to instantiate the subplugins with
     * Widget::addProperty.
     *
     * This is called _before_ a value is available to pupulate the context
     * menus plugin section.
     *
     * @param fieldDesc Description of the field
     * @param subpluginnames Array that is being filled with subplugin names
     *                       if supported. otherwise leave empty.
     * @return true if the field is handled in some way by this plugin
     */
    virtual bool probeProperty(FieldDescription *fieldDesc, std::vector<std::string> &subpluginnames) { return false; }

    /**
     * @brief Create a widget for this plugin
     *
     * The widget itself should not require Port/Type information, yet. It
     * may be instantiated without an associated port from the main menu.
     *
     * @return A widget handle that can be used to call functions on
     */
    virtual Widget *createWidget() { return nullptr; }
    /**
     * @brief Return the name
     *
     * @return A name to associate with the plugin
     */
    virtual std::string getName() = 0;
    /**
     * @brief Return flags
     *
     * @return Flags from enum Flags, 'or'ed together
     */
    virtual unsigned getFlags() = 0;

    /**
     * @brief Return a list of subplugin names that can be instantiated without fields
     *
     * @return A list of subplugin names
     */
    virtual std::vector<std::string> getStandaloneSubplugins() { return std::vector<std::string>(); }
};

/**
 * @brief This interface is used to interact with the actual widget
 *
 * This is created by Plugin::createWidget()
 */
class Widget : public QObject {
    Q_OBJECT;
public:
    /**
     * @brief The constructor
     *
     * @param parent A parent for the QObject if the plugin wishes to use it
     *               for object lifetime management
     */
    Widget(QObject *parent = nullptr) : QObject(parent) {}
    /**
     * @brief Retrieve the QWidget that can be embedded in a QMainWindow
     *
     * @return The QWidget
     */
    virtual QWidget *getWidget() { return nullptr; }
public slots:
    /**
     * @brief Add a Field from an OutputPort to the widget
     *
     * This is the last point where a widget can refuse to handle the field.
     *
     * In ruby vizkit land, this is equivalent to 'config'.
     *
     * Ownership of fieldDesc and its fields is not transfered and they can be
     * deleted after addOutputPortField returns. Use the type field
     * description from the value during update.
     *
     * The plugin may choose to ignore data from rock-display and instead
     * create its own reader here. A ValueConverter can be constructed by the
     * FieldDescription that helps extracting the fields data from other
     * Typelib::Value.
     *
     * @param fieldDesc A Field description to inform what kind of Field the
     *                  plugin wants to create
     * @param subpluginname A subplugin name from the set returned by
     *                      Plugin::probeOutputPort
     * @return A Field where updateOutputPort can be called to feed Values in.
     */
    virtual Field *addOutputPortField(FieldDescription const *fieldDesc, std::string const &subpluginname) { return nullptr; }
    /**
     * @brief Add a Field from an InputPort to the widget
     *
     * This is the last point where a widget can refuse to handle the field.
     *
     * Ownership of fieldDesc and its fields is not transfered and they can be
     * deleted after addInputPortField returns. Use the type field
     * description from the value during update.
     *
     * The plugin may choose to ignore data from rock-display and instead
     * create its own reader here. A ValueConverter can be constructed by the
     * FieldDescription that helps extracting the fields data from other
     * Typelib::Value.
     *
     * @param fieldDesc A Field description to inform what kind of Field the
     *                  plugin wants to create
     * @param subpluginname A subplugin name from the set returned by
     *                      Plugin::probeInputPort
     * @return A Field where updateInputPort can be called to feed Values in.
     */
    virtual Field *addInputPortField(FieldDescription const *fieldDesc, std::string const &subpluginname) { return nullptr; }
    /**
     * @brief Add a Field from a Property to the widget
     *
     * This is the last point where a widget can refuse to handle the field.
     *
     * Ownership of fieldDesc and its fields is not transfered and they can be
     * deleted after addPropertyField returns. Use the type field
     * description from the value during update.
     *
     * The plugin may choose to ignore data from rock-display and instead
     * create its own reader here. A ValueConverter can be constructed by the
     * FieldDescription that helps extracting the fields data from other
     * Typelib::Value.
     *
     * @param fieldDesc A Field description to inform what kind of Field the
     *                  plugin wants to create
     * @param subpluginname A subplugin name from the set returned by
     *                      Plugin::probeProperty
     * @return A Field where updateProperty can be called to feed Values in.
     */
    virtual Field *addPropertyField(FieldDescription const *fieldDesc, std::string const &subpluginname) { return nullptr; }
    /**
     * @brief Add a subplugin to the widget
     *
     * @param subpluginname The name of the subplugin from
     *                      Plugin::getStandaloneSubplugins
     * @return A reference that can be used to delete it again with
     *         removeStandaloneSubplugin
     */
    virtual StandaloneSubplugin *addStandaloneSubplugin(std::string const &subpluginname) { return nullptr; }
    /**
     * @brief Remove a Field from the widget
     *
     * The field was previously returned by addOutputPortField
     *
     * @param fieldDesc FieldDescription associated with the field
     * @param field Field returned by addOutputPortField
     */
    virtual void removeOutputPortField(FieldDescription const *fieldDesc, Field *field) { }
    /**
     * @brief Remove a Field from the widget
     *
     * The field was previously returned by addInputPortField
     *
     * @param fieldDesc FieldDescription associated with the field
     * @param field Field returned by addInputPortField
     */
    virtual void removeInputPortField(FieldDescription const *fieldDesc, Field *field) { }
    /**
     * @brief Remove a Field from the widget
     *
     * The field was previously returned by addPropertyField
     *
     * @param fieldDesc FieldDescription associated with the field
     * @param field Field returned by addPropertyField
     */
    virtual void removePropertyField(FieldDescription const *fieldDesc, Field *field) { }
    /**
     * @brief Remove a standalone Subplugin from the widget
     *
     * The plugin was previously returned by addStandaloneSubplugin
     *
     * @param plugin StandaloneSubplugin returned by addStandaloneSubplugin
     */
    virtual void removeStandaloneSubplugin(StandaloneSubplugin *plugin) { }
    /**
     * @brief Notifies when a tasks availability changes
     *
     * The field was previously returned by one of the add*Field methods.
     *
     * @param fieldDesc FieldDescription associated with the field
     * @param field Field returned by the add*Field method
     */
    virtual void taskAvailable(FieldDescription const *fieldDesc, Field *field,
                               bool available) { }
};

/**
 * @brief This interface provides entries for data transfer and a handle for removal of Fields from Widgets
 */
class Field : public QObject {
    Q_OBJECT;
public:
    /**
     * @brief The constructor
     *
     * @param parent A parent for the QObject if the plugin wishes to use it
     *               for object lifetime management
     */
    Field(QObject *parent = nullptr) : QObject(parent) {}
public slots:
    virtual void updateOutputPort(ValueHandle const *value) {}
    virtual void updateInputPort(ValueHandle *value) {} //this is called when the InputPort structures change their backing storage, or a value change by another plugin was done
    //ownership of value is not transfered, but value is guaranteed to always be the same object. value is not deleted before the VizkitWidget is deleted.
    virtual void updateProperty(ValueHandle *value) {} //this is called whenever the value of the thing changes. value is guaranteed to always be the same object. value is not deleted before the VizkitWidget is deleted.
};

/**
 * @brief This interface provides a handle for removal of StandaloneSubplugins from Widgets
 */
class StandaloneSubplugin  : public QObject {
    Q_OBJECT;
public:
    /**
     * @brief The constructor
     *
     * @param parent A parent for the QObject if the plugin wishes to use it
     *               for object lifetime management
     */
    StandaloneSubplugin(QObject *parent = nullptr) : QObject(parent) {}
};

/** @} */

/** @defgroup vizkitpluginprovided Class interfaces for objects provided by rock-display
 *  @{
 */

/* If you really need to pull out the heavy guns and need to read from the
 * port/property value to the field that you are supposed to interact with,
 * this is the tool that can be used.
 */

/**
 * @brief A class that can extract a field from a larger structure
 *
 * Only works if this can be wholly accomplished inside Typelib type system.
 *
 * This is created by FieldDescription::constructValueConverter
 */
class ValueConverter
{
public:
    virtual ~ValueConverter() = 0;
    /** Returns a reference into basevalue
     *
     * This only works if no further conversion is needed. otherwise,
     * a null Typelib::Value() is returned.
     *
     * @param basevalue The value from which to extract the field value
     * @return The field value
     */
    virtual Typelib::Value fieldValueFromBaseValue(Typelib::Value &basevalue) const = 0;
    /** This returns a pointer to a thing of getFieldTypeName() type.
     *
     * The data is owned by ValueConverter or basevalue. If you change it, you must call
     * rawFieldValueUpdated. Previous result values will be invalidated if basevalue changes.
     *
     * @param basevalue The value from which to extract the field value
     * @return A raw pointer to the field value
     */
    virtual void * rawFieldValueFromBaseValue(Typelib::Value &basevalue) const = 0;
    /** Possibly updates data in basevalue from fieldValue
     *
     * Ths fieldValue must be a result of the previous rawFieldValueFromBaseValue call with
     * the same basevalue.
     *
     * @param fieldValue A value as returned by rawFieldValueFromBaseValue
     * @param basevalue The basevalue that was used in the call to rawFieldValueFromBaseValue
     */
    virtual void rawFieldValueUpdated(void *fieldValue, Typelib::Value &basevalue) const = 0;
    /**
     * @brief Returns the base type that can be put into fieldValueFromBaseValue
     *
     * Only if it is backed by a Typelib Type.
     *
     * @return Returns the base type or nullptr
     */
    virtual Typelib::Type const *getBaseType() const = 0;
    /**
     * @brief Returns the field type that will be returned from fieldValueFromBaseValue
     *
     * Only if it is backed by a Typelib Type.
     *
     * @return Returns the field type or nullptr
     */
    virtual Typelib::Type const *getFieldType() const = 0;
    /**
     * @brief Returns a name representing the field inside the structure
     *
     * @return A string representing the field, roughly c-like
     */
    virtual std::string getFieldName() const = 0;
    /**
     * @brief Returns the type name of the base structure
     *
     * Result of getBaseType->getName() if applicable, or the name used by RTT
     *
     * @return The type name
     */
    virtual std::string getBaseTypeName() const = 0;
    /**
     * @brief Returns the type name of the field in the base structure
     *
     * Result of getFieldType->getName() if applicable, or the name used by RTT
     *
     * @return The type name
     */
    virtual std::string getFieldTypeName() const = 0;
};

/**
 * @brief Class that provides information about a field
 *
 * Port or Property it is from, TaskContext and NameService, Types, Registry.
 *
 * This avoids forcing inclusion of headers by also providing the names where possible.
 */
class FieldDescription
{
public:
    virtual ~FieldDescription() = 0;
    //only the getOutputPort, getInputPort, getProperty corresponding to
    //the value it is used with return non-null.

    /**
     * @brief Returns the OutputPort related to the Field, if any
     *
     * Only one of getOutputPort, getInputPort, getProperty return a non-nullptr
     *
     * @return The related OutputPort or nullptr
     */
    virtual RTT::base::OutputPortInterface *getOutputPort() const = 0;
    /**
     * @brief Returns the InputPort related to the Field, if any
     *
     * Only one of getOutputPort, getInputPort, getProperty return a non-nullptr
     *
     * @return The related InputPort or nullptr
     */
    virtual RTT::base::InputPortInterface *getInputPort() const = 0;
    /**
     * @brief Returns the Property related to the Field, if any
     *
     * Only one of getOutputPort, getInputPort, getProperty return a non-nullptr
     *
     * @return The related Property or nullptr
     */
    virtual RTT::base::PropertyBase *getProperty() const = 0;
    /**
     * @brief Returns the TaskContextProxy related to the Field
     *
     * @return The related TaskContextProxy
     */
    virtual RTT::corba::TaskContextProxy *getTaskContextProxy() const = 0;
    /**
     * @brief Returns the NameService related to the Field
     *
     * @return The related NameService
     */
    virtual orocos_cpp::NameService* getNameService() const = 0;
    /**
     * @brief Returns the type of the field
     *
     * @return The type if handled by Typelib, otherwise nullptr
     */
    virtual Typelib::Type const *getType() const = 0;
    /**
     * @brief Returns the Typelib::Registry related to the Field
     *
     * @return The related Registry
     */
    virtual const Typelib::Registry* getRegistry() const = 0;
    /**
     * @brief Convenience function providing the task name
     *
     * @return The taskname as provided by the TaskContextProxy from getTaskContextProxy()
     */
    virtual std::string getTaskName() const = 0;
    /**
     * @brief Convenience function providing the output port name
     *
     * Only the getOutputPortName, getInputPortName, getPropertyName corresponding to
     * the value it is used with return non-empty.
     *
     * @return The port name as provided by the Port from getOutputPort or empty string
     */
    virtual std::string getOutputPortName() const = 0;
    /**
     * @brief Convenience function providing the input port name
     *
     * Only the getOutputPortName, getInputPortName, getPropertyName corresponding to
     * the value it is used with return non-empty.
     *
     * @return The port name as provided by the Port from getInputPort or empty string
     */
    virtual std::string getInputPortName() const = 0;
    /**
     * @brief Convenience function providing the property name
     *
     * Only the getOutputPortName, getInputPortName, getPropertyName corresponding to
     * the value it is used with return non-empty.
     *
     * @return The property name as provided by the Property from getProperty or empty string
     */
    virtual std::string getPropertyName() const = 0;
    /**
     * @brief Returns a name representing the field inside the structure
     *
     * @return A string representing the field, roughly c-like
     */
    virtual std::string getFieldName() const = 0;
    /**
     * @brief Convenience function providing the type name
     *
     * @return The type derived from the Type from getType() oras used by RTT
     */
    virtual std::string getTypeName() const = 0;
    /** Constructs a ValueConverter
     *
     * Constructs a ValueConverter that can be used to create a Typelib::Value that
     * references the position inside a Typelib::Value that describes the whole port.
     * Ownership of the ValueConverter is passed to the caller. The construction
     * is potentially expensive, the ValueConverter can be reused for any value that is
     * read from the port.
     *
     * @return The ValueConverter that can produce this field from its base structure
     */
    virtual ValueConverter const *constructValueConverter() const = 0;
};

/**
 * @brief Interface that provides methods to obtain a value, information about its field and a way to notifiy about modifications
 */
class ValueHandle
{
public:
    virtual ~ValueHandle() = 0;
    /**
     * @brief Signals to rock-display that the value has been edited.
     *
     * Does nothing for values from OutputPorts
     *
     * @param forceSend  When the value is from an InputPort, immediately
     *                   sends the value to the port, otherwise just marks
     *                   it as changed in the GUI so the user can accept/
     *                   reject it. This is ignored for values from
     *                   Properties, where changes are immediately
     *                   transmitted.
     */
    virtual void edited(bool forceSend = true) = 0;
    /**
     * @brief Returns a FieldDescription describing the field of the value
     *
     * @return The FieldDescription
     */
    virtual FieldDescription *getFieldDescription() const = 0;
    /**
     * @brief Returns the value as Typelib::Value
     *
     * @return The Typelib::Value or a nullptr if not handled by Typelib
     */
    virtual Typelib::Value getValue() const = 0;
    /**
     * @brief Returns the value as a raw pointer
     *
     * @return A raw pointer to the value
     */
    virtual void const *getRawPtr() const = 0;
    /**
     * @brief Returns the value as a raw pointer
     *
     * @return A raw pointer to the value
     */
    virtual void *getRawPtr() = 0;
};

/** @} */

}
}

#define RockdisplayVizkitPluginFactory_iid "rock.rockdisplay.VizkitPluginFactory"

Q_DECLARE_INTERFACE(rockdisplay::vizkitplugin::PluginFactory, RockdisplayVizkitPluginFactory_iid)

#define ROCKDISPLAY_VIZKIT_PLUGINFACTORY(FactoryTypeName, PluginTypename) \
class FactoryTypeName : rockdisplay::vizkitplugin::PluginFactory { \
    Q_OBJECT; \
    Q_PLUGIN_METADATA(IID RockdisplayVizkitPluginFactory_iid) \
    Q_INTERFACES(rockdisplay::vizkitplugin::PluginFactory) \
\
public: \
    virtual std::vector<VizkitPlugin*> createPlugins() { \
        return {new PluginTypename();} \
    } \
};

#define ROCKDISPLAY_VIZKIT_SIMPLEPLUGINFACTORY(PluginTypename) ROCKDISPLAY_VIZKIT_PLUGINFACTORY(PluginTypename##PluginFactory, PluginTypename)

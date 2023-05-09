
#include "virtualjoystickplugin.hpp"
#include "virtualjoystickplugin_p.hpp"
#include <base/commands/Motion2D.hpp>
#include "designer/pluginmanager_p.h"
#include <QWidget>
#include <QtUiPlugin/customwidget.h>

using namespace rock_display;

/*
 * VirtualJoystick
 *   is a data source
 *   outputs /base/commands/Motion2D
 *   using signal axisChanged(double a,double b)
 *     conversion:
 *       sample.translation = x
 *       sample.rotation = (x==0 && y==0)?0:-abs(y)*atan2(y,abs(x))
 *     alternative conversion:
 *       sample.translation = a*options.maxspeed
 *       sample.rotation = -b*options.maxrotspeed
 *   should use a wrapper to provide configuration
 *   task availability is mapped to setEnabled (enabled property?)
 *
 * TODO: figure out a way to switch between the two conversions, preferably in
 *       a way that is consistent throughout all widgets
 */

VirtualJoystickInputPortField::VirtualJoystickInputPortField(QWidget *widget, QObject *parent)
    : rockdisplay::vizkitplugin::Field(parent), widget(widget), value(nullptr)
{
}

void VirtualJoystickInputPortField::updateInputPort(rockdisplay::vizkitplugin::ValueHandle *value)
{
    this->value = value;
}

void VirtualJoystickInputPortField::axisChanged(double x, double y) {
    /*
     * conversion:
     *   sample.translation = x
     *   sample.rotation = (x==0 && y==0)?0:-abs(y)*atan2(y,abs(x))
     * alternative conversion:
     *   sample.translation = a*options.maxspeed
     *   sample.rotation = -b*options.maxrotspeed
     */
    if (!value || !value->getRawPtr())
        return;
    {
        base::commands::Motion2D *v = reinterpret_cast<base::commands::Motion2D *>(value->getRawPtr());
        v->translation = x;
        v->rotation = (x == 0 && y == 0) ? 0 : -abs(y) * atan2(y, abs(x));
    }

    value->edited(true);
}

VirtualJoystickPropertyField::VirtualJoystickPropertyField(QWidget *widget, QObject *parent)
    : rockdisplay::vizkitplugin::Field(parent), widget(widget), value(nullptr)
{
}

void VirtualJoystickPropertyField::axisChanged(double x, double y) {
    if (!value || !value->getRawPtr())
        return;
    {
        base::commands::Motion2D *v = reinterpret_cast<base::commands::Motion2D *>(value->getRawPtr());
        v->translation = x;
        v->rotation = (x == 0 && y == 0) ? 0 : -abs(y) * atan2(y, abs(x));
    }

    value->edited();
}


void VirtualJoystickPropertyField::updateProperty(rockdisplay::vizkitplugin::ValueHandle *value)
{
    this->value = value;
}

VirtualJoystickWidget::VirtualJoystickWidget(QWidget *widget, QObject *parent)
 : rockdisplay::vizkitplugin::Widget(parent), widget(widget),
      inputportfield(nullptr),
      propertyfield(nullptr)
{
}

QWidget *VirtualJoystickWidget::getWidget()
{
    return widget;
}

rockdisplay::vizkitplugin::Field *VirtualJoystickWidget::addOutputPortField(const rockdisplay::vizkitplugin::FieldDescription *type, std::string const &subpluginname)
{
    return nullptr;
}

rockdisplay::vizkitplugin::Field *VirtualJoystickWidget::addInputPortField(const rockdisplay::vizkitplugin::FieldDescription *type, std::string const &subpluginname)
{
    if(inputportfield || propertyfield)
        return nullptr;
    inputportfield = new VirtualJoystickInputPortField(widget);

    connect(widget, SIGNAL(axisChanged(double,double)),
            inputportfield, SLOT(axisChanged(double,double)));

    return inputportfield;
}

rockdisplay::vizkitplugin::Field *VirtualJoystickWidget::addPropertyField(const rockdisplay::vizkitplugin::FieldDescription *type, std::string const &subpluginname)
{
    if(inputportfield || propertyfield)
        return nullptr;

    propertyfield = new VirtualJoystickPropertyField(widget);

    connect(widget, SIGNAL(axisChanged(double,double)),
            propertyfield, SLOT(axisChanged(double,double)));

    return propertyfield;
}

void VirtualJoystickWidget::taskAvailable(
    rockdisplay::vizkitplugin::FieldDescription const *fieldDesc,
    rockdisplay::vizkitplugin::Field *field,
    bool available)
{
    widget->setEnabled(available);
}

VirtualJoystickPlugin::VirtualJoystickPlugin()
    : widgetInterface(nullptr)
{
    widgetInterface = rockdisplay::QDesignerPluginManager::getInstance()->
                          findWidgetByName("VirtualJoystick");
    if (!widgetInterface)
        std::cerr << "Could not find a Qt Designer widget called \"VirtualJoystick\"" << std::endl;
}

bool VirtualJoystickPlugin::probeOutputPort(rockdisplay::vizkitplugin::FieldDescription *fieldDesc, std::vector<std::string> &names)
{
    return false;
}

bool VirtualJoystickPlugin::probeInputPort(rockdisplay::vizkitplugin::FieldDescription *fieldDesc, std::vector<std::string> &names)
{
    if(!widgetInterface)
        return false;
    return fieldDesc->getTypeName() == "/base/commands/Motion2D";
}

bool VirtualJoystickPlugin::probeProperty(rockdisplay::vizkitplugin::FieldDescription *fieldDesc, std::vector<std::string> &names)
{
    if(!widgetInterface)
        return false;
    return fieldDesc->getTypeName() == "/base/commands/Motion2D";
}

rockdisplay::vizkitplugin::Widget *VirtualJoystickPlugin::createWidget()
{
    if(!widgetInterface)
    {
        return nullptr;
    }
    QWidget *vjoy = widgetInterface->createWidget(nullptr);

    if (!vjoy)
    {
        return nullptr;
    }

    return new VirtualJoystickWidget(vjoy);
}

std::string VirtualJoystickPlugin::getName()
{
    return "VirtualJoystick";
}

unsigned VirtualJoystickPlugin::getFlags()
{
    return Flags::SingleFieldOnly;
}

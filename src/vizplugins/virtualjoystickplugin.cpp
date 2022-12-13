
#include "virtualjoystickplugin.hpp"
#include "virtualjoystickplugin_p.hpp"
#include <rock_widget_collection/RockWidgetCollection.h>
#include <base/commands/Motion2D.hpp>

using namespace rock_display;

/*
 *   using signal axisChanged(double a,double b)
 *     conversion:
 *       sample.translation = x
 *       sample.rotation = (x==0 && y==0)?0:-abs(y)*atan2(y,abs(x))
 *     alternative conversion:
 *       sample.translation = a*options.maxspeed
 *       sample.rotation = -b*options.maxrotspeed
 *   should use a wrapper to provide configuration
 *   task availability is mapped to setEnabled (enabled property?)
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

bool VirtualJoystickPlugin::probeOutputPort(rockdisplay::vizkitplugin::FieldDescription *fieldDesc, std::vector<std::string> &names)
{
    return false;
}

bool VirtualJoystickPlugin::probeInputPort(rockdisplay::vizkitplugin::FieldDescription *fieldDesc, std::vector<std::string> &names)
{
    return fieldDesc->getTypeName() == "/base/commands/Motion2D";
}

bool VirtualJoystickPlugin::probeProperty(rockdisplay::vizkitplugin::FieldDescription *fieldDesc, std::vector<std::string> &names)
{
    return fieldDesc->getTypeName() == "/base/commands/Motion2D";
}

rockdisplay::vizkitplugin::Widget *VirtualJoystickPlugin::createWidget()
{
    RockWidgetCollection collection;
    QList<QDesignerCustomWidgetInterface *> customWidgets = collection.customWidgets();

    QWidget *vjoy = nullptr;
    for (QDesignerCustomWidgetInterface *widgetInterface: customWidgets)
    {
        const std::string widgetName = widgetInterface->name().toStdString();

        if (widgetName == "VirtualJoystick")
        {
            vjoy = widgetInterface->createWidget(nullptr);
        }
    }

    if (!vjoy)
    {
        return nullptr;
    }
/*
    connect(vjoy, SIGNAL(axisChanged(double,double)),
            vjvh, SLOT(axisChanged(double,double)));
*/
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

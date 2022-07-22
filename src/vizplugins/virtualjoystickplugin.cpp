
#include "virtualjoystickplugin.hpp"
#include "virtualjoystickplugin_p.hpp"
#include <rock_widget_collection/RockWidgetCollection.h>
#include <typelib/typemodel.hh>
#include <base/commands/Motion2D.hpp>

VirtualJoystickPluginHandle::VirtualJoystickPluginHandle()
: PluginHandle("VirtualJoystick")
{
    typeName = "/base/commands/Motion2D";
}

bool VirtualJoystickPluginHandle::probe(Typelib::Type const &type, const Typelib::Registry* registry) const
{
    return type.getName() == typeName;
}

VizHandle *VirtualJoystickPluginHandle::createViz() const
{
    RockWidgetCollection collection;
    QList<QDesignerCustomWidgetInterface *> customWidgets = collection.customWidgets();

    QWidget *vjoy = nullptr;
    for (QDesignerCustomWidgetInterface *widgetInterface: customWidgets)
    {
        const std::string widgetName = widgetInterface->name().toStdString();

        if (widgetName == pluginName)
        {
            vjoy = widgetInterface->createWidget(nullptr);
        }
    }

    if (!vjoy)
    {
        return nullptr;
    }

    VirtualJoystickVizHandle *vjvh = new VirtualJoystickVizHandle;

    connect(vjoy, SIGNAL(axisChanged(double,double)),
            vjvh, SLOT(axisChanged(double,double)));

    vjoy->installEventFilter(vjvh);

    vjvh->widget = vjoy;
    vjvh->data = nullptr;
    return vjvh;
}

void VirtualJoystickVizHandle::updateEditable(void *data, RTT::base::DataSourceBase::shared_ptr base_sample)
{
    this->data = data;
    this->base_sample = base_sample;
}

void VirtualJoystickVizHandle::axisChanged(double x, double y)
{
    if(data) {
        base::commands::Motion2D *value = reinterpret_cast<base::commands::Motion2D *>(data);
        value->translation = x;
        value->rotation = (x == 0 && y == 0)?0:-abs(y) * atan2(y, abs(x));

        emit editableChanged(data, base_sample, true);
    }
}

bool VirtualJoystickVizHandle::eventFilter(QObject *obj, QEvent *event) {
    if (event->type() == QEvent::Close) {
        emit closing(this);
    }
    return false;
}


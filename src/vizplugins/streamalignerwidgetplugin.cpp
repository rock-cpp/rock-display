
#include "streamalignerwidgetplugin.hpp"
#include "streamalignerwidgetplugin_p.hpp"
#include "designer/pluginmanager_p.h"
#include <QWidget>
#include <QtUiPlugin/customwidget.h>
#include <iostream>

/*
 * StreamAlignerWidget
 *   is a data sink
 *   inputs /aggregator/StreamAlignerStatus
 *   uses updateData(sample)
 */

using namespace rock_display;

StreamAlignerWidgetWidget::StreamAlignerWidgetWidget(QWidget *widget, QMetaMethod method, QObject *parent)
: rockdisplay::vizkitplugin::Widget(parent), widget(widget),
method(method),
outputportfield(new StreamAlignerWidgetOutputPortField(widget, method)),
inputportfield(new StreamAlignerWidgetInputPortField(widget, method)),
propertyfield(new StreamAlignerWidgetPropertyField(widget, method))
{
}

QWidget *StreamAlignerWidgetWidget::getWidget()
{
    return widget;
}

rockdisplay::vizkitplugin::Field *StreamAlignerWidgetWidget::addOutputPortField( const rockdisplay::vizkitplugin::FieldDescription * type, std::string const &subpluginname )
{
    return outputportfield;
}

rockdisplay::vizkitplugin::Field *StreamAlignerWidgetWidget::addInputPortField( const rockdisplay::vizkitplugin::FieldDescription * type, std::string const &subpluginname )
{
    return inputportfield;
}

rockdisplay::vizkitplugin::Field *StreamAlignerWidgetWidget::addPropertyField( const rockdisplay::vizkitplugin::FieldDescription * type, std::string const &subpluginname )
{
    return propertyfield;
}

StreamAlignerWidgetOutputPortField::StreamAlignerWidgetOutputPortField(QWidget *widget, QMetaMethod method, QObject *parent)
: rockdisplay::vizkitplugin::Field(parent), widget(widget),
method(method)
{
}

void StreamAlignerWidgetOutputPortField::updateOutputPort(rockdisplay::vizkitplugin::ValueHandle const *data)
{
    QGenericArgument val("void *", data->getRawPtr());

    method.invoke(widget, val);
}

StreamAlignerWidgetInputPortField::StreamAlignerWidgetInputPortField(QWidget *widget, QMetaMethod method, QObject *parent)
: rockdisplay::vizkitplugin::Field(parent), widget(widget),
method(method)
{
}

void StreamAlignerWidgetInputPortField::updateInputPort(rockdisplay::vizkitplugin::ValueHandle *data)
{
    QGenericArgument val("void *", data->getRawPtr());

    method.invoke(widget, val);
}

StreamAlignerWidgetPropertyField::StreamAlignerWidgetPropertyField(QWidget *widget, QMetaMethod method, QObject *parent)
: rockdisplay::vizkitplugin::Field(parent), widget(widget),
method(method)
{
}

void StreamAlignerWidgetPropertyField::updateProperty(rockdisplay::vizkitplugin::ValueHandle *data)
{
    QGenericArgument val("void *", data->getRawPtr());

    method.invoke(widget, val);
}

StreamAlignerWidgetPlugin::StreamAlignerWidgetPlugin()
    : widgetInterface(nullptr)
{
    widgetInterface = rockdisplay::QDesignerPluginManager::getInstance()->
                          findWidgetByName("StreamAlignerWidget");
    if (!widgetInterface)
        std::cerr << "Could not find a Qt Designer widget called \"StreamAlignerWidget\"" << std::endl;
}

bool StreamAlignerWidgetPlugin::probeOutputPort(rockdisplay::vizkitplugin::FieldDescription *fieldDesc, std::vector<std::string> &names)
{
    if(!widgetInterface)
        return false;
    return fieldDesc->getTypeName() == "/aggregator/StreamAlignerStatus";
}

bool StreamAlignerWidgetPlugin::probeInputPort(rockdisplay::vizkitplugin::FieldDescription *fieldDesc, std::vector<std::string> &names) {
    if(!widgetInterface)
        return false;
    return fieldDesc->getTypeName() == "/aggregator/StreamAlignerStatus";
}

bool StreamAlignerWidgetPlugin::probeProperty(rockdisplay::vizkitplugin::FieldDescription *fieldDesc, std::vector<std::string> &names) {
    if(!widgetInterface)
        return false;
    return fieldDesc->getTypeName() == "/aggregator/StreamAlignerStatus";
}

rockdisplay::vizkitplugin::Widget *StreamAlignerWidgetPlugin::createWidget()
{
    if(!widgetInterface)
    {
        return nullptr;
    }
    QWidget *saView = widgetInterface->createWidget(nullptr);

    if (!saView)
    {
        return nullptr;
    }

    const QMetaObject *metaPlugin = saView->metaObject();

    QMetaMethod found_method;
    for(int i = 0 ; i < metaPlugin->methodCount(); i++)
    {
        QMetaMethod method = metaPlugin->method(i);
        auto parameterList = method.parameterTypes();
        if(parameterList.size() != 1)
        {
            continue;
        }
        if (parameterList[0].toStdString() != "aggregator::StreamAlignerStatus")
        {
            continue;
        }

        std::string signature = method.methodSignature().toStdString();
        std::string methodStr("updateData");
        if (signature.size() < methodStr.size() || signature.substr(0, methodStr.size()) != methodStr)
        {
            continue;
        }
        found_method = method;
    }

    StreamAlignerWidgetWidget *ivvh = new StreamAlignerWidgetWidget(saView, found_method);

    return ivvh;
}

std::string StreamAlignerWidgetPlugin::getName()
{
    return "StreamAlignerWidget";
}

unsigned StreamAlignerWidgetPlugin::getFlags() {
    //In principle, this is compatible with CanRemoveFields, but there
    //is no way to tell the widget itself about it, so the field just
    //stops to update.
    return KeepOpenWithoutFields | PreferSingleWidget |
           WidgetCanHandleMultipleFields;
}


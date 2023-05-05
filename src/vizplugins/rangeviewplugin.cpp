
#include "rangeviewplugin.hpp"
#include "rangeviewplugin_p.hpp"
#include <base/samples/LaserScan.hpp>
#include "designer/pluginmanager_p.h"
#include <QWidget>
#include <QtUiPlugin/customwidget.h>
#include <iostream>

/*
 * RangeView:
 *   is a data sink
 *   inputs /base/samples/LaserScan
 *   uses setRangeScan3(range_scan.ranges:p
 *     [p * cos(index*range_scan.angular_resolution), p * sin(index*range_scan.angular_resolution), 0 ])
 */

using namespace rock_display;

RangeViewWidget::RangeViewWidget(
    QWidget *widget,
    QMetaMethod const &method, QObject *parent)
    : rockdisplay::vizkitplugin::Widget(parent), widget(widget),
      method(method),
      outputportfield(new RangeViewOutputPortField(widget, method)),
      inputportfield(new RangeViewInputPortField(widget, method)),
      propertyfield(new RangeViewPropertyField(widget, method))
{
}

QWidget *RangeViewWidget::getWidget()
{
    return widget;
}

rockdisplay::vizkitplugin::Field *RangeViewWidget::addOutputPortField( const rockdisplay::vizkitplugin::FieldDescription * type, std::string const &subpluginname )
{
    return outputportfield;
}

rockdisplay::vizkitplugin::Field *RangeViewWidget::addInputPortField( const rockdisplay::vizkitplugin::FieldDescription * type, std::string const &subpluginname )
{
    return inputportfield;
}

rockdisplay::vizkitplugin::Field *RangeViewWidget::addPropertyField( const rockdisplay::vizkitplugin::FieldDescription * type, std::string const &subpluginname )
{
    return propertyfield;
}

//helper function to concentrate the common code
static void doUpdate(rockdisplay::vizkitplugin::ValueHandle const *data,
                     QWidget *widget,
                     QMetaMethod const &method)
{
    //range_scan.ranges:p
    //  [p * cos(index*range_scan.angular_resolution), p * sin(index*range_scan.angular_resolution), 0 ]
    base::samples::LaserScan const * range_scan = reinterpret_cast<base::samples::LaserScan const *>(data->getRawPtr());
    QList<double> converted;

    for(unsigned i = 0; i < range_scan->ranges.size(); i++) {
        auto &p = range_scan->ranges[i];
        converted.push_back(p * cos(i * range_scan->angular_resolution));
        converted.push_back(p * sin(i * range_scan->angular_resolution));
        converted.push_back(0.0);
    }

    QGenericArgument convertedval("QList<double>", &converted);
    if (!convertedval.data())
    {
        return;
    }

    method.invoke(widget, convertedval);
}

RangeViewOutputPortField::RangeViewOutputPortField(
    QWidget *widget, QMetaMethod const &method, QObject *parent)
    : rockdisplay::vizkitplugin::Field(parent), widget(widget),
      method(method)
{
}

void RangeViewOutputPortField::updateOutputPort(
    rockdisplay::vizkitplugin::ValueHandle const *data)
{
    doUpdate(data, widget, method);
}

RangeViewInputPortField::RangeViewInputPortField(
    QWidget *widget, QMetaMethod const &method, QObject *parent)
    : rockdisplay::vizkitplugin::Field(parent), widget(widget),
      method(method)
{
}

void RangeViewInputPortField::updateInputPort(rockdisplay::vizkitplugin::ValueHandle *data)
{
    doUpdate(data, widget, method);
}

RangeViewPropertyField::RangeViewPropertyField(
    QWidget *widget, QMetaMethod const &method, QObject *parent)
    : rockdisplay::vizkitplugin::Field(parent), widget(widget),
      method(method)
{
}

void RangeViewPropertyField::updateProperty(rockdisplay::vizkitplugin::ValueHandle *data)
{
    doUpdate(data, widget, method);
}

RangeViewPlugin::RangeViewPlugin()
    : widgetInterface(nullptr)
{
    widgetInterface = rockdisplay::QDesignerPluginManager::getInstance()->
                      findWidgetByName("RangeView");
    if (!widgetInterface)
        std::cerr << "Could not find a Qt Designer widget called \"RangeView\"" << std::endl;
}

bool RangeViewPlugin::probeOutputPort(rockdisplay::vizkitplugin::FieldDescription *fieldDesc, std::vector<std::string> &names)
{
    if(!widgetInterface)
        return false;
    return fieldDesc->getTypeName() == "/base/samples/LaserScan";
}

bool RangeViewPlugin::probeInputPort(rockdisplay::vizkitplugin::FieldDescription *fieldDesc, std::vector<std::string> &names) {
    if(!widgetInterface)
        return false;
    return fieldDesc->getTypeName() == "/base/samples/LaserScan";
}

bool RangeViewPlugin::probeProperty(rockdisplay::vizkitplugin::FieldDescription *fieldDesc, std::vector<std::string> &names) {
    if(!widgetInterface)
        return false;
    return fieldDesc->getTypeName() == "/base/samples/LaserScan";
}

rockdisplay::vizkitplugin::Widget *RangeViewPlugin::createWidget()
{
    if(!widgetInterface)
    {
        return nullptr;
    }
    QWidget *artfHorz = widgetInterface->createWidget(nullptr);

    if (!artfHorz)
    {
        return nullptr;
    }

    const QMetaObject *metaPlugin = artfHorz->metaObject();

    QMetaMethod found_method;
    for(int i = 0 ; i < metaPlugin->methodCount(); i++)
    {
        QMetaMethod method = metaPlugin->method(i);
        auto parameterList = method.parameterTypes();
        if(parameterList.size() != 1)
        {
            continue;
        }

        std::string signature = method.methodSignature().toStdString();
        std::string methodStr("setRangeScan3");
        if (signature.size() >= methodStr.size() &&
            signature.substr(0, methodStr.size()) == methodStr)
        {
            found_method = method;
        }
    }

    RangeViewWidget *ahvh = new RangeViewWidget(artfHorz, found_method);

    return ahvh;
}

std::string RangeViewPlugin::getName()
{
    return "RangeView";
}

unsigned RangeViewPlugin::getFlags() {
    return Flags::SingleFieldOnly;
}



#include "sonarwidgetplugin.hpp"
#include "sonarwidgetplugin_p.hpp"
#include "designer/pluginmanager_p.h"
#include <QWidget>
#include <QtUiPlugin/customwidget.h>
#include <iostream>

/*
 * SonarWidget:
 *   is a data sink
 *   inputs /base/samples/SonarScan
 *   inputs /base/samples/Sonar
 *   uses setData(sample), honoring types.
 */

using namespace rock_display;

SonarWidgetWidget::SonarWidgetWidget(
    QWidget *widget,
    QMetaMethod const &Sonar_method,
    QMetaMethod const &SonarScan_method, QObject *parent)
    : rockdisplay::vizkitplugin::Widget(parent), widget(widget),
      Sonar_method(Sonar_method), SonarScan_method(SonarScan_method),
      outputportfield(new SonarWidgetOutputPortField(widget, Sonar_method, SonarScan_method)),
      inputportfield(new SonarWidgetInputPortField(widget, Sonar_method, SonarScan_method)),
      propertyfield(new SonarWidgetPropertyField(widget, Sonar_method, SonarScan_method))
{
}

QWidget *SonarWidgetWidget::getWidget()
{
    return widget;
}

rockdisplay::vizkitplugin::Field *SonarWidgetWidget::addOutputPortField( const rockdisplay::vizkitplugin::FieldDescription * type, std::string const &subpluginname )
{
    return outputportfield;
}

rockdisplay::vizkitplugin::Field *SonarWidgetWidget::addInputPortField( const rockdisplay::vizkitplugin::FieldDescription * type, std::string const &subpluginname )
{
    return inputportfield;
}

rockdisplay::vizkitplugin::Field *SonarWidgetWidget::addPropertyField( const rockdisplay::vizkitplugin::FieldDescription * type, std::string const &subpluginname )
{
    return propertyfield;
}

//helper function to concentrate the common code
static void doUpdate(rockdisplay::vizkitplugin::ValueHandle const *data,
                     QWidget *widget,
                     QMetaMethod const &Sonar_method,
                     QMetaMethod const &SonarScan_method)
{
    QGenericArgument dataval("void", data->getRawPtr());
    if(!dataval.data())
    {
        return;
    }
    if (data->getFieldDescription()->getTypeName() == "/base/samples/SonarScan")
    {
        SonarScan_method.invoke(widget, dataval);
    }
    else if (data->getFieldDescription()->getTypeName() == "/base/samples/Sonar")
    {
        Sonar_method.invoke(widget, dataval);
    }
}

SonarWidgetOutputPortField::SonarWidgetOutputPortField(
    QWidget *widget, QMetaMethod const &Sonar_method,
    QMetaMethod const &SonarScan_method, QObject *parent)
    : rockdisplay::vizkitplugin::Field(parent), widget(widget),
      Sonar_method(Sonar_method),
      SonarScan_method(SonarScan_method)
{
}

void SonarWidgetOutputPortField::updateOutputPort(
    rockdisplay::vizkitplugin::ValueHandle const *data)
{
    doUpdate(data, widget, Sonar_method, SonarScan_method);
}

SonarWidgetInputPortField::SonarWidgetInputPortField(
    QWidget *widget, QMetaMethod const &Sonar_method,
    QMetaMethod const &SonarScan_method, QObject *parent)
    : rockdisplay::vizkitplugin::Field(parent), widget(widget),
      Sonar_method(Sonar_method), SonarScan_method(SonarScan_method)
{
}

void SonarWidgetInputPortField::updateInputPort(rockdisplay::vizkitplugin::ValueHandle *data)
{
    doUpdate(data, widget, Sonar_method, SonarScan_method);
}

SonarWidgetPropertyField::SonarWidgetPropertyField(
    QWidget *widget, QMetaMethod const &Sonar_method,
    QMetaMethod const &SonarScan_method, QObject *parent)
: rockdisplay::vizkitplugin::Field(parent), widget(widget),
Sonar_method(Sonar_method), SonarScan_method(SonarScan_method)
{
}

void SonarWidgetPropertyField::updateProperty(rockdisplay::vizkitplugin::ValueHandle *data)
{
    doUpdate(data, widget, Sonar_method, SonarScan_method);
}

SonarWidgetPlugin::SonarWidgetPlugin()
    : widgetInterface(nullptr)
{
    widgetInterface = rockdisplay::QDesignerPluginManager::getInstance()->
                      findWidgetByName("SonarWidget");
    if (!widgetInterface)
        std::cerr << "Could not find a Qt Designer widget called \"SonarWidget\"" << std::endl;
}

bool SonarWidgetPlugin::probeOutputPort(rockdisplay::vizkitplugin::FieldDescription *fieldDesc, std::vector<std::string> &names)
{
    if(!widgetInterface)
        return false;
    return fieldDesc->getTypeName() == "/base/RigidBodyState" ||
           fieldDesc->getTypeName() == "/base/samples/RigidBodyState" ||
           fieldDesc->getTypeName() == "/base/Pose" ||
           fieldDesc->getTypeName() == "/base/Orientation" ||
           fieldDesc->getTypeName() == "/Eigen/Quarternion";
}

bool SonarWidgetPlugin::probeInputPort(rockdisplay::vizkitplugin::FieldDescription *fieldDesc, std::vector<std::string> &names) {
    if(!widgetInterface)
        return false;
    return fieldDesc->getTypeName() == "/base/RigidBodyState" ||
           fieldDesc->getTypeName() == "/base/samples/RigidBodyState" ||
           fieldDesc->getTypeName() == "/base/Pose" ||
           fieldDesc->getTypeName() == "/base/Orientation" ||
           fieldDesc->getTypeName() == "/Eigen/Quarternion";
}

bool SonarWidgetPlugin::probeProperty(rockdisplay::vizkitplugin::FieldDescription *fieldDesc, std::vector<std::string> &names) {
    if(!widgetInterface)
        return false;
    return fieldDesc->getTypeName() == "/base/RigidBodyState" ||
           fieldDesc->getTypeName() == "/base/samples/RigidBodyState" ||
           fieldDesc->getTypeName() == "/base/Pose" ||
           fieldDesc->getTypeName() == "/base/Orientation" ||
           fieldDesc->getTypeName() == "/Eigen/Quarternion";
}

rockdisplay::vizkitplugin::Widget *SonarWidgetPlugin::createWidget()
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

    QMetaMethod found_Sonar_method;
    QMetaMethod found_SonarScan_method;
    for(int i = 0 ; i < metaPlugin->methodCount(); i++)
    {
        QMetaMethod method = metaPlugin->method(i);
        auto parameterList = method.parameterTypes();
        if(parameterList.size() != 1)
        {
            continue;
        }
        if(parameterList[0].toStdString() != "base::samples::Sonar" &&
            parameterList[0].toStdString() != "base::samples::SonarScan")
        {
            continue;
        }

        std::string signature = method.methodSignature().toStdString();
        std::string methodStr("setData");
        if (signature.size() < methodStr.size() ||
            signature.substr(0, methodStr.size()) != methodStr)
        {
            continue;
        }
        if (parameterList[0].toStdString() == "base::samples::Sonar")
        {
            found_Sonar_method = method;
        }
        if (parameterList[0].toStdString() == "base::samples::SonarScan")
        {
            found_SonarScan_method = method;
        }
    }

    SonarWidgetWidget *ahvh = new SonarWidgetWidget(artfHorz, found_Sonar_method,
            found_SonarScan_method);

    return ahvh;
}

std::string SonarWidgetPlugin::getName()
{
    return "SonarWidget";
}

unsigned SonarWidgetPlugin::getFlags() {
    return Flags::SingleFieldOnly;
}


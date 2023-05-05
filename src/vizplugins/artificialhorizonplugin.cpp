
#include "artificialhorizonplugin.hpp"
#include "artificialhorizonplugin_p.hpp"
#include <base/samples/RigidBodyState.hpp>
#include "designer/pluginmanager_p.h"
#include <QWidget>
#include <QtUiPlugin/customwidget.h>

/*
 * ArtificialHorizon
 *   seems to be superseded by compass widget, probably now known as orientation
 *   is a data sink
 *   inputs /wrappers/RigidBodyState
 *   inputs /wrappers/samples/RigidBodyState
 *   inputs /wrappers/Pose
 *     uses orientation member
 *   inputs Eigen::Quaternion (not sure about the typelib type)
 *     sample: /wrappers/Orientation
 *       sample = Eigen::Quaternion.new(sample.re, *sample.im.to_a)
 *   inputs /wrappers/Orientation
 *   conversion:
 *     setPitchAngle(sample.pitch)
 *     setRollAngle(sample.roll)
 */

using namespace rock_display;

ArtificialHorizonWidget::ArtificialHorizonWidget(
    QWidget *widget,
    QMetaMethod const &setPitchAngle_method,
    QMetaMethod const &setRollAngle_method, QObject *parent)
    : rockdisplay::vizkitplugin::Widget(parent), widget(widget),
      setPitchAngle_method(setPitchAngle_method), setRollAngle_method(setRollAngle_method),
      outputportfield(new ArtificialHorizonOutputPortField(widget, setPitchAngle_method, setRollAngle_method)),
      inputportfield(new ArtificialHorizonInputPortField(widget, setPitchAngle_method, setRollAngle_method)),
      propertyfield(new ArtificialHorizonPropertyField(widget, setPitchAngle_method, setRollAngle_method))
{
}

QWidget *ArtificialHorizonWidget::getWidget()
{
    return widget;
}

rockdisplay::vizkitplugin::Field *ArtificialHorizonWidget::addOutputPortField( const rockdisplay::vizkitplugin::FieldDescription * type, std::string const &subpluginname )
{
    return outputportfield;
}

rockdisplay::vizkitplugin::Field *ArtificialHorizonWidget::addInputPortField( const rockdisplay::vizkitplugin::FieldDescription * type, std::string const &subpluginname )
{
    return inputportfield;
}

rockdisplay::vizkitplugin::Field *ArtificialHorizonWidget::addPropertyField( const rockdisplay::vizkitplugin::FieldDescription * type, std::string const &subpluginname )
{
    return propertyfield;
}

static void doUpdate(double pitch, double roll,
                     QWidget *widget,
                     QMetaMethod const &setPitchAngle_method,
                     QMetaMethod const &setRollAngle_method)
{
    QGenericArgument pitchval("double", &pitch);
    QGenericArgument rollval("double", &roll);
    if (!pitchval.data() || !rollval.data())
    {
        return;
    }
    setPitchAngle_method.invoke(widget, pitchval);
    setRollAngle_method.invoke(widget, rollval);
}

template<typename T>
static void doUpdateTyped(T const &data,
                     QWidget *widget,
                     QMetaMethod const &setPitchAngle_method,
                     QMetaMethod const &setRollAngle_method);

template<>
void doUpdateTyped(base::samples::RigidBodyState const &data,
                     QWidget *widget,
                     QMetaMethod const &setPitchAngle_method,
                     QMetaMethod const &setRollAngle_method)
{
    doUpdate(data.getPitch(), data.getRoll(),
             widget, setPitchAngle_method, setRollAngle_method);
}

template<>
void doUpdateTyped(base::Quaterniond const &data,
                     QWidget *widget,
                     QMetaMethod const &setPitchAngle_method,
                     QMetaMethod const &setRollAngle_method)
{
    //lifted from base/types ruby bindings
    float pitch;
    float roll;
    const Eigen::Matrix3d m = data.toRotationMatrix();
    double i = Eigen::Vector2d(m.coeff(2, 2), m.coeff(2, 1)).norm();
    double y = atan2(-m.coeff(2, 0), i);
    if (isnan(i))
    {
        pitch = NAN;
        roll = NAN;
    }
    else if (i > Eigen::NumTraits<double>::dummy_precision())
    {
        //double x = ::atan2(m.coeff(1, 0), m.coeff(0, 0));
        double z = ::atan2(m.coeff(2, 1), m.coeff(2, 2));
        pitch = y;
        roll = z;
    }
    else
    {
        double z =
            (m.coeff(2, 0) > 0 ? 1 : -1) * ::atan2(-m.coeff(0, 1), m.coeff(1, 1));
        pitch = y;
        roll = z;
    }
    doUpdate(pitch, roll,
             widget, setPitchAngle_method, setRollAngle_method);
}

template<>
void doUpdateTyped(base::Pose const &data,
                     QWidget *widget,
                     QMetaMethod const &setPitchAngle_method,
                     QMetaMethod const &setRollAngle_method)
{
    doUpdateTyped(data.orientation,
                  widget, setPitchAngle_method, setRollAngle_method);
}

//helper function to concentrate the common code
static void doUpdate(rockdisplay::vizkitplugin::ValueHandle const *data,
                     QWidget *widget,
                     QMetaMethod const &setPitchAngle_method,
                     QMetaMethod const &setRollAngle_method)
{
    if (data->getFieldDescription()->getTypeName() == "/base/samples/RigidBodyState" ||
            data->getFieldDescription()->getTypeName() == "/base/RigidBodyState")
    {
        doUpdateTyped(
            *reinterpret_cast<base::samples::RigidBodyState  const *>(
                data->getRawPtr()),
            widget, setPitchAngle_method, setRollAngle_method);
    }
    else if (data->getFieldDescription()->getTypeName() == "/base/Pose")
    {
        doUpdateTyped(
            *reinterpret_cast<base::Pose const *>(data->getRawPtr()),
            widget, setPitchAngle_method, setRollAngle_method);
    }
    else if (data->getFieldDescription()->getTypeName() == "/base/Orientation" ||
             data->getFieldDescription()->getTypeName() == "/base/Quaterniond" ||
             data->getFieldDescription()->getTypeName() == "/Eigen/Quaternion<double>")
    {
        doUpdateTyped(
            *reinterpret_cast<base::Quaterniond const *>(data->getRawPtr()),
            widget, setPitchAngle_method, setRollAngle_method);
    }
}

ArtificialHorizonOutputPortField::ArtificialHorizonOutputPortField(
    QWidget *widget, QMetaMethod const &setPitchAngle_method,
    QMetaMethod const &setRollAngle_method, QObject *parent)
    : rockdisplay::vizkitplugin::Field(parent), widget(widget),
      setPitchAngle_method(setPitchAngle_method),
      setRollAngle_method(setRollAngle_method)
{
}

void ArtificialHorizonOutputPortField::updateOutputPort(
    rockdisplay::vizkitplugin::ValueHandle const *data)
{
    doUpdate(data, widget, setPitchAngle_method, setRollAngle_method);
}

ArtificialHorizonInputPortField::ArtificialHorizonInputPortField(
    QWidget *widget, QMetaMethod const &setPitchAngle_method,
    QMetaMethod const &setRollAngle_method, QObject *parent)
    : rockdisplay::vizkitplugin::Field(parent), widget(widget),
      setPitchAngle_method(setPitchAngle_method), setRollAngle_method(setRollAngle_method)
{
}

void ArtificialHorizonInputPortField::updateInputPort(rockdisplay::vizkitplugin::ValueHandle *data)
{
    doUpdate(data, widget, setPitchAngle_method, setRollAngle_method);
}

ArtificialHorizonPropertyField::ArtificialHorizonPropertyField(
    QWidget *widget, QMetaMethod const &setPitchAngle_method,
    QMetaMethod const &setRollAngle_method, QObject *parent)
: rockdisplay::vizkitplugin::Field(parent), widget(widget),
setPitchAngle_method(setPitchAngle_method), setRollAngle_method(setRollAngle_method)
{
}

void ArtificialHorizonPropertyField::updateProperty(rockdisplay::vizkitplugin::ValueHandle *data)
{
    doUpdate(data, widget, setPitchAngle_method, setRollAngle_method);
}

ArtificialHorizonPlugin::ArtificialHorizonPlugin()
    : widgetInterface(nullptr)
{
    widgetInterface = rockdisplay::QDesignerPluginManager::getInstance()->
                      findWidgetByName("ArtificialHorizon");
    if (!widgetInterface)
        std::cerr << "Could not find a Qt Designer widget called \"ArtificialHorizon\"" << std::endl;
}

bool ArtificialHorizonPlugin::probeOutputPort(rockdisplay::vizkitplugin::FieldDescription *fieldDesc, std::vector<std::string> &names)
{
    if(!widgetInterface)
        return false;
    return fieldDesc->getTypeName() == "/base/RigidBodyState" ||
           fieldDesc->getTypeName() == "/base/samples/RigidBodyState" ||
           fieldDesc->getTypeName() == "/base/Pose" ||
           fieldDesc->getTypeName() == "/base/Orientation" ||
           fieldDesc->getTypeName() == "/Eigen/Quarternion";
}

bool ArtificialHorizonPlugin::probeInputPort(rockdisplay::vizkitplugin::FieldDescription *fieldDesc, std::vector<std::string> &names) {
    if(!widgetInterface)
        return false;
    return fieldDesc->getTypeName() == "/base/RigidBodyState" ||
           fieldDesc->getTypeName() == "/base/samples/RigidBodyState" ||
           fieldDesc->getTypeName() == "/base/Pose" ||
           fieldDesc->getTypeName() == "/base/Orientation" ||
           fieldDesc->getTypeName() == "/Eigen/Quarternion";
}

bool ArtificialHorizonPlugin::probeProperty(rockdisplay::vizkitplugin::FieldDescription *fieldDesc, std::vector<std::string> &names) {
    if(!widgetInterface)
        return false;
    return fieldDesc->getTypeName() == "/base/RigidBodyState" ||
           fieldDesc->getTypeName() == "/base/samples/RigidBodyState" ||
           fieldDesc->getTypeName() == "/base/Pose" ||
           fieldDesc->getTypeName() == "/base/Orientation" ||
           fieldDesc->getTypeName() == "/Eigen/Quarternion";
}

rockdisplay::vizkitplugin::Widget *ArtificialHorizonPlugin::createWidget()
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

    QMetaMethod found_setPitchAngle_method;
    QMetaMethod found_setRollAngle_method;
    for(int i = 0 ; i < metaPlugin->methodCount(); i++)
    {
        QMetaMethod method = metaPlugin->method(i);
        auto parameterList = method.parameterTypes();
        if(parameterList.size() != 1)
        {
            continue;
        }

        std::string signature = method.methodSignature().toStdString();
        std::string setPitchAngle_methodStr("setPitchAngle");
        std::string setRollAngle_methodStr("setRollAngle");
        if (signature.size() >= setPitchAngle_methodStr.size() &&
            signature.substr(0, setPitchAngle_methodStr.size()) == setPitchAngle_methodStr)
        {
            found_setPitchAngle_method = method;
        }
        if (signature.size() >= setRollAngle_methodStr.size() &&
            signature.substr(0, setRollAngle_methodStr.size()) == setRollAngle_methodStr)
        {
            found_setRollAngle_method = method;
        }
    }

    ArtificialHorizonWidget *ahvh = new ArtificialHorizonWidget(artfHorz, found_setPitchAngle_method,
            found_setRollAngle_method);

    return ahvh;
}

std::string ArtificialHorizonPlugin::getName()
{
    return "ArtificialHorizon";
}

unsigned ArtificialHorizonPlugin::getFlags() {
    return Flags::SingleFieldOnly;
}


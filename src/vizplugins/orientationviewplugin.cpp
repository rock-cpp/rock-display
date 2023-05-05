
#include "orientationviewplugin.hpp"
#include "orientationviewplugin_p.hpp"
#include <base/samples/RigidBodyState.hpp>
#include "designer/pluginmanager_p.h"
#include <QWidget>
#include <QtUiPlugin/customwidget.h>

/*
 * OrientationView:
 *   is a data sink
 *   inputs /base/samples/RigidBodyState
 *   inputs /base/Pose
 *     uses orientation member
 *   inputs /base/Orientation
 *   inputs Eigen::Quaternion (not sure about the typelib type)
 *     sample: /wrappers/Orientation
 *       sample = Eigen::Quaternion.new(sample.re, *sample.im.to_a)
 *   conversion:
 *     setPitchAngle(sample.pitch)
 *     setRollAngle(sample.roll)
 *     setHeadingAngle(sample.yaw)
 */

using namespace rock_display;

OrientationViewWidget::OrientationViewWidget(
    QWidget *widget,
    QMetaMethod const &setPitchAngle_method,
    QMetaMethod const &setRollAngle_method,
    QMetaMethod const &setHeadingAngle_method,
    QObject *parent)
    : rockdisplay::vizkitplugin::Widget(parent), widget(widget),
      setPitchAngle_method(setPitchAngle_method),
      setRollAngle_method(setRollAngle_method),
      setHeadingAngle_method(setHeadingAngle_method),
      outputportfield(new OrientationViewOutputPortField(
                          widget, setPitchAngle_method, setRollAngle_method,
                          setHeadingAngle_method)),
      inputportfield(new OrientationViewInputPortField(
                         widget, setPitchAngle_method, setRollAngle_method,
                         setHeadingAngle_method)),
      propertyfield(new OrientationViewPropertyField(
                        widget, setPitchAngle_method, setRollAngle_method,
                        setHeadingAngle_method))
{
}

QWidget *OrientationViewWidget::getWidget()
{
    return widget;
}

rockdisplay::vizkitplugin::Field *OrientationViewWidget::addOutputPortField( const rockdisplay::vizkitplugin::FieldDescription * type, std::string const &subpluginname )
{
    return outputportfield;
}

rockdisplay::vizkitplugin::Field *OrientationViewWidget::addInputPortField( const rockdisplay::vizkitplugin::FieldDescription * type, std::string const &subpluginname )
{
    return inputportfield;
}

rockdisplay::vizkitplugin::Field *OrientationViewWidget::addPropertyField( const rockdisplay::vizkitplugin::FieldDescription * type, std::string const &subpluginname )
{
    return propertyfield;
}

static void doUpdate(double pitch, double roll, double yaw,
                     QWidget *widget,
                     QMetaMethod const &setPitchAngle_method,
                     QMetaMethod const &setRollAngle_method,
                     QMetaMethod const &setHeadingAngle_method)
{
    fprintf(stderr, "Have P/R/Y %g %g %g\n",pitch, roll, yaw);
    QGenericArgument pitchval("double", &pitch);
    QGenericArgument rollval("double", &roll);
    QGenericArgument yawval("double", &yaw);
    if (!pitchval.data() || !rollval.data() || !yawval.data())
    {
        return;
    }
    setPitchAngle_method.invoke(widget, pitchval);
    setRollAngle_method.invoke(widget, rollval);
    setHeadingAngle_method.invoke(widget, yawval);
}

template<typename T>
static void doUpdateTyped(T const &data,
                     QWidget *widget,
                     QMetaMethod const &setPitchAngle_method,
                     QMetaMethod const &setRollAngle_method,
                     QMetaMethod const &setHeadingAngle_method);

template<>
void doUpdateTyped(base::samples::RigidBodyState const &data,
                     QWidget *widget,
                     QMetaMethod const &setPitchAngle_method,
                     QMetaMethod const &setRollAngle_method,
                     QMetaMethod const &setHeadingAngle_method)
{
    doUpdate(data.getPitch(), data.getRoll(), data.getYaw(), widget,
             setPitchAngle_method, setRollAngle_method, setHeadingAngle_method);
}

template<>
void doUpdateTyped(base::Quaterniond const &data,
                     QWidget *widget,
                     QMetaMethod const &setPitchAngle_method,
                     QMetaMethod const &setRollAngle_method,
                     QMetaMethod const &setHeadingAngle_method)
{
    //lifted from base/types ruby bindings
    float pitch;
    float roll;
    float yaw;
    const Eigen::Matrix3d m = data.toRotationMatrix();
    double i = Eigen::Vector2d(m.coeff(2, 2), m.coeff(2, 1)).norm();
    double y = atan2(-m.coeff(2, 0), i);
    if (isnan(i))
    {
        pitch = NAN;
        roll = NAN;
        yaw = NAN;
    }
    else if (i > Eigen::NumTraits<double>::dummy_precision())
    {
        double x = ::atan2(m.coeff(1, 0), m.coeff(0, 0));
        double z = ::atan2(m.coeff(2, 1), m.coeff(2, 2));
        pitch = y;
        roll = z;
        yaw = x;
    }
    else
    {
        double z =
            (m.coeff(2, 0) > 0 ? 1 : -1) * ::atan2(-m.coeff(0, 1), m.coeff(1, 1));
        pitch = y;
        roll = z;
        yaw = 0;
    }
    doUpdate(pitch, roll, yaw, widget,
             setPitchAngle_method, setRollAngle_method, setHeadingAngle_method);
}

template<>
void doUpdateTyped(base::Pose const &data,
                   QWidget *widget,
                   QMetaMethod const &setPitchAngle_method,
                   QMetaMethod const &setRollAngle_method,
                   QMetaMethod const &setHeadingAngle_method)
{
    doUpdateTyped(data.orientation, widget, setPitchAngle_method,
                  setRollAngle_method, setHeadingAngle_method);
}

//helper function to concentrate the common code
static void doUpdate(rockdisplay::vizkitplugin::ValueHandle const *data,
                     QWidget *widget,
                     QMetaMethod const &setPitchAngle_method,
                     QMetaMethod const &setRollAngle_method,
                     QMetaMethod const &setHeadingAngle_method)
{
    if (data->getFieldDescription()->getTypeName() == "/base/samples/RigidBodyState" ||
            data->getFieldDescription()->getTypeName() == "/base/RigidBodyState")
    {
        doUpdateTyped(
            *reinterpret_cast<base::samples::RigidBodyState  const *>(
                data->getRawPtr()),
            widget, setPitchAngle_method, setRollAngle_method,
            setHeadingAngle_method);
    }
    else if (data->getFieldDescription()->getTypeName() == "/base/Pose")
    {
        doUpdateTyped(
            *reinterpret_cast<base::Pose const *>(data->getRawPtr()),
            widget, setPitchAngle_method, setRollAngle_method,
            setHeadingAngle_method);
    }
    else if (data->getFieldDescription()->getTypeName() == "/base/Orientation" ||
             data->getFieldDescription()->getTypeName() == "/base/Quaterniond" ||
             data->getFieldDescription()->getTypeName() == "/Eigen/Quaternion<double>")
    {
        doUpdateTyped(
            *reinterpret_cast<base::Quaterniond const *>(data->getRawPtr()),
            widget, setPitchAngle_method, setRollAngle_method,
            setHeadingAngle_method);
    }
}

OrientationViewOutputPortField::OrientationViewOutputPortField(
    QWidget *widget, QMetaMethod const &setPitchAngle_method,
    QMetaMethod const &setRollAngle_method,
    QMetaMethod const &setHeadingAngle_method, QObject *parent)
    : rockdisplay::vizkitplugin::Field(parent), widget(widget),
      setPitchAngle_method(setPitchAngle_method),
      setRollAngle_method(setRollAngle_method),
      setHeadingAngle_method(setHeadingAngle_method)
{
}

void OrientationViewOutputPortField::updateOutputPort(rockdisplay::vizkitplugin::ValueHandle const *data)
{
    doUpdate(data, widget, setPitchAngle_method, setRollAngle_method,
             setHeadingAngle_method);
}

OrientationViewInputPortField::OrientationViewInputPortField(
    QWidget *widget, QMetaMethod const &setPitchAngle_method,
    QMetaMethod const &setRollAngle_method,
    QMetaMethod const &setHeadingAngle_method, QObject *parent)
    : rockdisplay::vizkitplugin::Field(parent), widget(widget),
      setPitchAngle_method(setPitchAngle_method),
      setRollAngle_method(setRollAngle_method),
      setHeadingAngle_method(setHeadingAngle_method)
{
}

void OrientationViewInputPortField::updateInputPort(rockdisplay::vizkitplugin::ValueHandle *data)
{
    doUpdate(data, widget, setPitchAngle_method, setRollAngle_method,
        setHeadingAngle_method);
}

OrientationViewPropertyField::OrientationViewPropertyField(
    QWidget *widget, QMetaMethod const &setPitchAngle_method,
    QMetaMethod const &setRollAngle_method,
    QMetaMethod const &setHeadingAngle_method, QObject *parent)
    : rockdisplay::vizkitplugin::Field(parent), widget(widget),
      setPitchAngle_method(setPitchAngle_method),
      setRollAngle_method(setRollAngle_method),
      setHeadingAngle_method(setHeadingAngle_method)
{
}

void OrientationViewPropertyField::updateProperty(rockdisplay::vizkitplugin::ValueHandle *data)
{
    doUpdate(data, widget, setPitchAngle_method, setRollAngle_method,
        setHeadingAngle_method);
}

OrientationViewPlugin::OrientationViewPlugin()
    : widgetInterface(nullptr)
{
    widgetInterface = rockdisplay::QDesignerPluginManager::getInstance()->
                          findWidgetByName("OrientationView");
    if (!widgetInterface)
        std::cerr << "Could not find a Qt Designer widget called \"OrientationView\"" << std::endl;
}

bool OrientationViewPlugin::probeOutputPort(rockdisplay::vizkitplugin::FieldDescription *fieldDesc, std::vector<std::string> &names)
{
    if(!widgetInterface)
        return false;
    return fieldDesc->getTypeName() == "/base/RigidBodyState" ||
           fieldDesc->getTypeName() == "/base/samples/RigidBodyState" ||
           fieldDesc->getTypeName() == "/base/Pose" ||
           fieldDesc->getTypeName() == "/base/Orientation" ||
           fieldDesc->getTypeName() == "/Eigen/Quarternion";
}

bool OrientationViewPlugin::probeInputPort(rockdisplay::vizkitplugin::FieldDescription *fieldDesc, std::vector<std::string> &names) {
    if(!widgetInterface)
        return false;
    return fieldDesc->getTypeName() == "/base/RigidBodyState" ||
           fieldDesc->getTypeName() == "/base/samples/RigidBodyState" ||
           fieldDesc->getTypeName() == "/base/Pose" ||
           fieldDesc->getTypeName() == "/base/Orientation" ||
           fieldDesc->getTypeName() == "/Eigen/Quarternion";
}

bool OrientationViewPlugin::probeProperty(rockdisplay::vizkitplugin::FieldDescription *fieldDesc, std::vector<std::string> &names) {
    if(!widgetInterface)
        return false;
    return fieldDesc->getTypeName() == "/base/RigidBodyState" ||
           fieldDesc->getTypeName() == "/base/samples/RigidBodyState" ||
           fieldDesc->getTypeName() == "/base/Pose" ||
           fieldDesc->getTypeName() == "/base/Orientation" ||
           fieldDesc->getTypeName() == "/Eigen/Quarternion";
}

rockdisplay::vizkitplugin::Widget *OrientationViewPlugin::createWidget()
{
    if(!widgetInterface)
    {
        return nullptr;
    }
    QWidget *oView = widgetInterface->createWidget(nullptr);

    if (!oView)
    {
        return nullptr;
    }

    const QMetaObject *metaPlugin = oView->metaObject();

    QMetaMethod found_setPitchAngle_method;
    QMetaMethod found_setRollAngle_method;
    QMetaMethod found_setHeadingAngle_method;
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
        std::string setHeadingAngle_methodStr("setHeadingAngle");
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
        if (signature.size() >= setHeadingAngle_methodStr.size() &&
            signature.substr(0, setHeadingAngle_methodStr.size()) == setHeadingAngle_methodStr)
        {
            found_setHeadingAngle_method = method;
        }
    }

    OrientationViewWidget *ovvh = new OrientationViewWidget(oView, found_setPitchAngle_method,
            found_setRollAngle_method, found_setHeadingAngle_method);

    return ovvh;
}

std::string OrientationViewPlugin::getName()
{
    return "OrientationView";
}

unsigned OrientationViewPlugin::getFlags() {
    return Flags::SingleFieldOnly;
}


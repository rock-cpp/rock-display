
#include "sonarviewplugin.hpp"
#include "sonarviewplugin_p.hpp"
#include <base/samples/Sonar.hpp>
#include <base/samples/SonarBeam.hpp>
#include <iostream>
#include "designer/pluginmanager_p.h"
#include <QWidget>
#include <QtUiPlugin/customwidget.h>

/*
 * SonarView:
 *  is a data sink
 *  inputs /sensorData/Sonar   // commented
 *  inputs /base/samples/SonarScan  // commented
 *  inputs /base/samples/SonarBeam
 *  uses setSonarScan and conversion
 *
 *
 *   if !defined? @init
 *     @options ||= default_options
 *     openGL true
 *     @time_overlay_object = addText(-150,-5,0,"time")
 *     @time_overlay_object.setColor(Qt::Color.new(255,255,0))
 *     @time_overlay_object.setPosFactor(1,1);
 *     @time_overlay_object.setBackgroundColor(Qt::Color.new(0,0,0,40))
 *     @init = true
 *   end
 *
 *   if @options[:time_overlay] == true
 *     if sonar_scan.respond_to?(:time) && sonar_scan.time.instance_of?(Time)
 *       time = sonar_scan.time
 *     end
 *     @time_overlay_object.setText(time.strftime("%b %d %Y %H:%M:%S"))
 *   end
 *
 *   #TODO very very ugly
 *   #a new visualization is planed
 *   #that would fix the problem
 *   #if sonar_scan.class.name == "/base/samples/SonarScan"
 *   #  setSonarScan(sonar_scan.scanData.to_byte_array[8..-1], sonar_scan.scanData.size, sonar_scan.angle, sonar_scan.time_beetween_bins, false)
 *   if sonar_scan.class.name == "/sensorData/Sonar"
 *     setSonarScan(sonar_scan.scanData.to_byte_array[8..-1], sonar_scan.scanData.size, sonar_scan.bearing, sonar_scan.adInterval, true)
 *   elsif sonar_scan.class.name == "/base/samples/SonarBeam"
 *       angle = if sonar_scan.bearing.rad < 0
 *                   -sonar_scan.bearing.rad
 *               else
 *                   2* Math::PI - sonar_scan.bearing.rad
 *               end
 *   setSonarScan(sonar_scan.beam.to_byte_array[8..-1], sonar_scan.beam.size, angle, sonar_scan.sampling_interval, false)
 *   else
 *       STDERR.puts "Cannot Handle Data of type: #{sonar_scan.class.name}, please check vizkit/ruby/lib/vizkit/cplusplus_extensions/sonar_view.rb"
 *   end
 *   update2()
 *
 */

using namespace rock_display;

SonarViewWidget::SonarViewWidget(QWidget *widget, QMetaMethod addText_method, QMetaMethod setSonarScan_method, QMetaMethod update2_method, QObject *parent)
    : rockdisplay::vizkitplugin::Widget(parent), widget(widget),
      addText_method(addText_method), setSonarScan_method(setSonarScan_method),
      update2_method(update2_method),
      outputportfield(new SonarViewOutputPortField(this)),
      inputportfield(new SonarViewInputPortField(this)),
      propertyfield(new SonarViewPropertyField(this)),
      init(false), time_overlay_object(nullptr)
{
}

QWidget *SonarViewWidget::getWidget()
{
    return widget;
}

rockdisplay::vizkitplugin::Field *SonarViewWidget::addOutputPortField( const rockdisplay::vizkitplugin::FieldDescription * type, std::string const &subpluginname )
{
    return outputportfield;
}

rockdisplay::vizkitplugin::Field *SonarViewWidget::addInputPortField( const rockdisplay::vizkitplugin::FieldDescription * type, std::string const &subpluginname )
{
    return inputportfield;
}

rockdisplay::vizkitplugin::Field *SonarViewWidget::addPropertyField( const rockdisplay::vizkitplugin::FieldDescription * type, std::string const &subpluginname )
{
    return propertyfield;
}

void SonarViewWidget::doUpdate(rockdisplay::vizkitplugin::ValueHandle const *data) {
    if(!init) {
        addText_method.invoke(widget, Q_RETURN_ARG(QObject *, time_overlay_object),
                              Q_ARG(int, -150), Q_ARG(int, -5), Q_ARG(int, 0), Q_ARG(QString, "time"));
        time_overlay_object->metaObject()->invokeMethod(time_overlay_object,
                "setColor", Q_ARG(QColor, QColor(255, 255, 0)));
        time_overlay_object->metaObject()->invokeMethod(time_overlay_object,
                "setPosFactor", Q_ARG(float, 1), Q_ARG(float, 1));
        time_overlay_object->metaObject()->invokeMethod(time_overlay_object,
                "setBackgroundColor", Q_ARG(QColor, QColor(0,0,0,40)));
        init = true;
    }
    base::samples::SonarBeam const *sample =
        reinterpret_cast<base::samples::SonarBeam const *>(
            data->getRawPtr());
    if(false) { //@options[:time_overlay] == true
        base::Time time;
        time = sample->time;
        //time.strftime("%b %d %Y %H:%M:%S")
        time_overlay_object->metaObject()->invokeMethod(time_overlay_object,
                "setText", Q_ARG(QString, QString::fromStdString(time.toString())));
    }
    float angle = -sample->bearing.rad;
    if(angle < 0)
        angle += M_PI * 2;
    //void setSonarScan(const char *data, int size, double angle,double timeBetweenBins ,bool fromBearing=false)
    setSonarScan_method.invoke(
        widget, Q_ARG(const char *, (const char *)sample->beam.data() + 8),
        Q_ARG(int, sample->beam.size()), Q_ARG(double, angle),
        Q_ARG(double, sample->sampling_interval),
        Q_ARG(bool, false));
    update2_method.invoke(widget);
}

SonarViewOutputPortField::SonarViewOutputPortField(SonarViewWidget *widget, QObject *parent)
: rockdisplay::vizkitplugin::Field(parent), widget(widget)
{
}

void SonarViewOutputPortField::updateOutputPort(rockdisplay::vizkitplugin::ValueHandle const *data)
{
    widget->doUpdate(data);
}

SonarViewInputPortField::SonarViewInputPortField(SonarViewWidget *widget, QObject *parent)
: rockdisplay::vizkitplugin::Field(parent), widget(widget)
{
}

void SonarViewInputPortField::updateInputPort(rockdisplay::vizkitplugin::ValueHandle *data)
{
    widget->doUpdate(data);
}

SonarViewPropertyField::SonarViewPropertyField(SonarViewWidget *widget, QObject *parent)
: rockdisplay::vizkitplugin::Field(parent), widget(widget)
{
}

void SonarViewPropertyField::updateProperty(rockdisplay::vizkitplugin::ValueHandle *data)
{
    widget->doUpdate(data);
}

SonarViewPlugin::SonarViewPlugin()
    : widgetInterface(nullptr)
{
    widgetInterface = rockdisplay::QDesignerPluginManager::getInstance()->
                      findWidgetByName("SonarView");
    if (!widgetInterface)
        std::cerr << "Could not find a Qt Designer widget called \"SonarView\"" << std::endl;
}

bool SonarViewPlugin::probeOutputPort(rockdisplay::vizkitplugin::FieldDescription *fieldDesc, std::vector<std::string> &names)
{
    if(!widgetInterface)
        return false;
    return fieldDesc->getTypeName() == "/base/samples/SonarBeam";
}

bool SonarViewPlugin::probeInputPort(rockdisplay::vizkitplugin::FieldDescription *fieldDesc, std::vector<std::string> &names) {
    if(!widgetInterface)
        return false;
    return fieldDesc->getTypeName() == "/base/samples/SonarBeam";
}

bool SonarViewPlugin::probeProperty(rockdisplay::vizkitplugin::FieldDescription *fieldDesc, std::vector<std::string> &names) {
    if(!widgetInterface)
        return false;
    return fieldDesc->getTypeName() == "/base/samples/SonarBeam";
}

rockdisplay::vizkitplugin::Widget *SonarViewPlugin::createWidget()
{
    if(!widgetInterface)
    {
        return nullptr;
    }
    QWidget *imView = widgetInterface->createWidget(nullptr);

    if (!imView)
    {
        return nullptr;
    }

    const QMetaObject *metaPlugin = imView->metaObject();

    QMetaMethod found_addText_method;
    QMetaMethod found_setSonarScan_method;
    QMetaMethod found_update2_method;
    for(int i = 0 ; i < metaPlugin->methodCount(); i++)
    {
        QMetaMethod method = metaPlugin->method(i);
        auto parameterList = method.parameterTypes();

        std::string signature = method.methodSignature().toStdString();
        std::string addText_methodStr("addText");
        std::string setSonarScan_methodStr("setSonarScan");
        std::string update2_methodStr("update2");
        if (signature.size() >= addText_methodStr.size() && signature.substr(0, addText_methodStr.size()) == addText_methodStr)
        {
            found_addText_method = method;
        }
        if (signature.size() >= setSonarScan_methodStr.size() && signature.substr(0, setSonarScan_methodStr.size()) == setSonarScan_methodStr)
        {
            found_setSonarScan_method = method;
        }
        if (signature.size() >= update2_methodStr.size() && signature.substr(0, update2_methodStr.size()) == update2_methodStr)
        {
            found_update2_method = method;
        }
    }

    SonarViewWidget *ivvh = new SonarViewWidget(imView, found_addText_method,
            found_setSonarScan_method, found_update2_method);

    return ivvh;
}

std::string SonarViewPlugin::getName()
{
    return "SonarView";
}

unsigned SonarViewPlugin::getFlags() {
    return Flags::SingleFieldOnly;
}


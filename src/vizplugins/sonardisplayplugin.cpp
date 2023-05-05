
#include "sonardisplayplugin.hpp"
#include "sonardisplayplugin_p.hpp"
#include <base/samples/SonarBeam.hpp>
#include <iostream>
#include "designer/pluginmanager_p.h"
#include <QWidget>
#include <QtUiPlugin/customwidget.h>

/*
 * SonarDisplay:
 *   is a data sink
 *   inputs /base/samples/SonarBeam
 *   uses addSonarBeam, setUpSonar and conversion
 *       @resolution ||= 0.1
 *       @number_of_bins ||= 100
 *
 *       angle = sonar_beam.bearing.rad + Math::PI*0.5
 *       angle +=  Math::PI*2 if angle < 0
 *       data = sonar_beam.beam.to_byte_array[8..-1]
 *       resolution = sonar_beam.sampling_interval*sonar_beam.speed_of_sound*0.5
 *
 *       if(@resolution != resolution || @number_of_bins < data.size)
 *           beamwidth_vertical = if sonar_beam.beamwidth_vertical == 0
 *                                    30.0/180*Math::PI
 *                                else
 *                                    sonar_beam.beamwidth_vertical
 *                                end
 *           setUpSonar(72,data.size, 5.0/180*Math::PI,resolution,beamwidth_vertical)
 *           @number_of_bins = data.size
 *           @resolution = resolution
 *       end
 *
 *       addSonarBeam(angle,data.size,data)
 */

using namespace rock_display;

SonarDisplayWidget::SonarDisplayWidget(QWidget *widget,
                                       QMetaMethod addSonarBeam_method,
                                       QMetaMethod setUpSonar_method,
                                       QObject *parent)
    : rockdisplay::vizkitplugin::Widget(parent), widget(widget),
      addSonarBeam_method(addSonarBeam_method), setUpSonar_method(setUpSonar_method),
      outputportfield(new SonarDisplayOutputPortField(this)),
      inputportfield(new SonarDisplayInputPortField(this)),
      propertyfield(new SonarDisplayPropertyField(this)),
      number_of_bins(100),
      resolution(0.1)
{
}

QWidget *SonarDisplayWidget::getWidget()
{
    return widget;
}

rockdisplay::vizkitplugin::Field *SonarDisplayWidget::addOutputPortField( const rockdisplay::vizkitplugin::FieldDescription * type, std::string const &subpluginname )
{
    return outputportfield;
}

rockdisplay::vizkitplugin::Field *SonarDisplayWidget::addInputPortField( const rockdisplay::vizkitplugin::FieldDescription * type, std::string const &subpluginname )
{
    return inputportfield;
}

rockdisplay::vizkitplugin::Field *SonarDisplayWidget::addPropertyField( const rockdisplay::vizkitplugin::FieldDescription * type, std::string const &subpluginname )
{
    return propertyfield;
}

void SonarDisplayWidget::updateData(rockdisplay::vizkitplugin::ValueHandle const *data)
{
    base::samples::SonarBeam const *sonar_beam = reinterpret_cast<base::samples::SonarBeam const *>(data->getRawPtr());

    double angle = sonar_beam->bearing.rad + M_PI*0.5;
    if(angle < 0)
        angle += M_PI*2;
    std::vector<uint8_t> subdata(sonar_beam->beam.begin()+8, sonar_beam->beam.end());

    double resolution = sonar_beam->sampling_interval*sonar_beam->speed_of_sound*0.5;
    if(this->resolution != resolution || this->number_of_bins != subdata.size()) {
        float beamwidth_vertical = sonar_beam->beamwidth_vertical;
        if(abs(beamwidth_vertical) <= 1e-6)
            beamwidth_vertical = 30.0/180*M_PI;
        //  void setUpSonar(int number_of_beams, int number_of_bins,float horizontal_resolution,
        //         float distance_resolution, float vertical_resolution);
        setUpSonar_method.invoke(widget, Q_ARG(int, 72),
                                 Q_ARG(int, subdata.size()),
                                 Q_ARG(float, 5.0 / 180 * M_PI),
                                 Q_ARG(float, resolution),
                                 Q_ARG(float, beamwidth_vertical));
        this->number_of_bins = subdata.size();
        this->resolution = resolution;
    }
    //  void addSonarBeam(float bearing,int number_of_bins,const char* pbuffer);
    addSonarBeam_method.invoke(
        widget, Q_ARG(float, angle), Q_ARG(int, subdata.size()),
        Q_ARG(char const *, reinterpret_cast<char const *>(subdata.data())));
}

SonarDisplayOutputPortField::SonarDisplayOutputPortField(SonarDisplayWidget *widget, QObject *parent)
: rockdisplay::vizkitplugin::Field(parent), widget(widget)
{
}

void SonarDisplayOutputPortField::updateOutputPort(rockdisplay::vizkitplugin::ValueHandle const *data)
{
    widget->updateData(data);
}

SonarDisplayInputPortField::SonarDisplayInputPortField(SonarDisplayWidget *widget, QObject *parent)
    : rockdisplay::vizkitplugin::Field(parent), widget(widget)
{
}

void SonarDisplayInputPortField::updateInputPort(rockdisplay::vizkitplugin::ValueHandle *data)
{
    widget->updateData(data);
}

SonarDisplayPropertyField::SonarDisplayPropertyField(SonarDisplayWidget *widget, QObject *parent)
    : rockdisplay::vizkitplugin::Field(parent), widget(widget)
{
}

void SonarDisplayPropertyField::updateProperty(rockdisplay::vizkitplugin::ValueHandle *data)
{
    widget->updateData(data);
}

SonarDisplayPlugin::SonarDisplayPlugin()
    : widgetInterface(nullptr)
{
    widgetInterface = rockdisplay::QDesignerPluginManager::getInstance()->
                      findWidgetByName("SonarDisplay");
    if (!widgetInterface)
        std::cerr << "Could not find a Qt Designer widget called \"SonarDisplay\"" << std::endl;
}

bool SonarDisplayPlugin::probeOutputPort(rockdisplay::vizkitplugin::FieldDescription *fieldDesc, std::vector<std::string> &names)
{
    if(!widgetInterface)
        return false;
    return fieldDesc->getTypeName() == "/base/samples/SonarBeam";
}

bool SonarDisplayPlugin::probeInputPort(rockdisplay::vizkitplugin::FieldDescription *fieldDesc, std::vector<std::string> &names) {
    if(!widgetInterface)
        return false;
    return fieldDesc->getTypeName() == "/base/samples/SonarBeam";
}

bool SonarDisplayPlugin::probeProperty(rockdisplay::vizkitplugin::FieldDescription *fieldDesc, std::vector<std::string> &names) {
    if(!widgetInterface)
        return false;
    return fieldDesc->getTypeName() == "/base/samples/SonarBeam";
}

rockdisplay::vizkitplugin::Widget *SonarDisplayPlugin::createWidget()
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

    QMetaMethod found_setUpSonar_method;
    QMetaMethod found_addSonarBeam_method;
    for(int i = 0 ; i < metaPlugin->methodCount(); i++)
    {
        QMetaMethod method = metaPlugin->method(i);
        auto parameterList = method.parameterTypes();
        if(parameterList.size() != 1)
        {
            continue;
        }

        std::string signature = method.methodSignature().toStdString();
        std::string addSonarBeam_methodStr("addSonarBeam");
        std::string setUpSonar_methodStr("setUpSonar");
        if (signature.size() >= addSonarBeam_methodStr.size() && signature.substr(0, addSonarBeam_methodStr.size()) == addSonarBeam_methodStr)
        {
            found_addSonarBeam_method = method;
        }
        if (signature.size() >= setUpSonar_methodStr.size() && signature.substr(0, setUpSonar_methodStr.size()) == setUpSonar_methodStr)
        {
            found_setUpSonar_method = method;
        }
    }

    SonarDisplayWidget *ivvh = new SonarDisplayWidget(imView, found_addSonarBeam_method,
            found_setUpSonar_method);

    return ivvh;
}

std::string SonarDisplayPlugin::getName()
{
    return "SonarDisplay";
}

unsigned SonarDisplayPlugin::getFlags() {
    return Flags::SingleFieldOnly;
}



#include "imageviewplugin.hpp"
#include "imageviewplugin_p.hpp"
#include <base/samples/Frame.hpp>
#include <base/samples/DistanceImage.hpp>
#include <iostream>
#include "designer/pluginmanager_p.h"
#include <QWidget>
#include <QtUiPlugin/customwidget.h>

/*
 * ImageView
 *   is a data sink
 *   inputs /base/samples/DistanceImage using display
 *     calls setFrame
 *   inputs /base/samples/frame/Frame using display
 *     calls setFrame
 *   inputs /base/samples/frame/FramePair using display2
 *     that calls to display to display just one, determined by option
 *   optionally displays a time code in bottom right corner
 *
 * TODO figure out how to do options with rock-display (for the time code
 *   in this case)
 */

using namespace rock_display;

ImageViewWidget::ImageViewWidget(QWidget *widget, QMetaMethod frame_method, QMetaMethod distanceimage_method, QObject *parent)
: rockdisplay::vizkitplugin::Widget(parent), widget(widget),
frame_method(frame_method), distanceimage_method(distanceimage_method),
outputportfield(new ImageViewOutputPortField(this)),
inputportfield(new ImageViewInputPortField(this)),
propertyfield(new ImageViewPropertyField(this))
{
}

QWidget *ImageViewWidget::getWidget()
{
    return widget;
}

rockdisplay::vizkitplugin::Field *ImageViewWidget::addOutputPortField( const rockdisplay::vizkitplugin::FieldDescription * type, std::string const &subpluginname )
{
    return outputportfield;
}

rockdisplay::vizkitplugin::Field *ImageViewWidget::addInputPortField( const rockdisplay::vizkitplugin::FieldDescription * type, std::string const &subpluginname )
{
    return inputportfield;
}

rockdisplay::vizkitplugin::Field *ImageViewWidget::addPropertyField( const rockdisplay::vizkitplugin::FieldDescription * type, std::string const &subpluginname )
{
    return propertyfield;
}

void ImageViewWidget::doUpdate(const rockdisplay::vizkitplugin::ValueHandle *data) {
    QMetaMethod method;
    QGenericArgument val;

    base::Time time;
    if (data->getFieldDescription()->getTypeName() == "/base/samples/frame/Frame")
    {
        method = frame_method;
        val = QGenericArgument("void *", data->getRawPtr());
        base::samples::frame::Frame const *sample = reinterpret_cast<base::samples::frame::Frame const *>(data->getRawPtr());
        time = sample->time;
    }
    else if (data->getFieldDescription()->getTypeName() == "/base/samples/DistanceImage")
    {
        method = distanceimage_method;
        val = QGenericArgument("void *", data->getRawPtr());
        base::samples::DistanceImage const *sample = reinterpret_cast<base::samples::DistanceImage const *>(data->getRawPtr());
        time = sample->time;
    }
    else if (data->getFieldDescription()->getTypeName() == "/base/samples/frame/FramePair")
    {
        base::samples::frame::FramePair const *v =
            reinterpret_cast<base::samples::frame::FramePair  const *>(
                data->getRawPtr());
        bool useFirst = true;//TODO options
        method = frame_method;
        if (useFirst)
        {
            val = QGenericArgument("void *", &v->first);
        }
        else
        {
            val = QGenericArgument("void *", &v->second);
        }
        time = v->time;
    }
    else
    {
        return;
    }
    if (!val.data())
    {
        return;
    }

    if(false) // @options[:time_overlay]
    {
        widget->metaObject()->invokeMethod(
            widget, "addText",
            Q_ARG(QString, QString::fromStdString(time.toString())),
            Q_ARG(int, 3),
            Q_ARG(QColor, Qt::black),
            Q_ARG(bool, false));
    }

    method.invoke(widget, val);
}

ImageViewOutputPortField::ImageViewOutputPortField(ImageViewWidget *widget, QObject *parent)
: rockdisplay::vizkitplugin::Field(parent), widget(widget)
{
}

void ImageViewOutputPortField::updateOutputPort(rockdisplay::vizkitplugin::ValueHandle const *data)
{
    widget->doUpdate(data);
}

ImageViewInputPortField::ImageViewInputPortField(ImageViewWidget *widget, QObject *parent)
: rockdisplay::vizkitplugin::Field(parent), widget(widget)
{
}

void ImageViewInputPortField::updateInputPort(rockdisplay::vizkitplugin::ValueHandle *data)
{
    widget->doUpdate(data);
}

ImageViewPropertyField::ImageViewPropertyField(ImageViewWidget *widget, QObject *parent)
: rockdisplay::vizkitplugin::Field(parent), widget(widget)
{
}

void ImageViewPropertyField::updateProperty(rockdisplay::vizkitplugin::ValueHandle *data)
{
    widget->doUpdate(data);
}

ImageViewPlugin::ImageViewPlugin()
    : widgetInterface(nullptr)
{
    widgetInterface = rockdisplay::QDesignerPluginManager::getInstance()->
                      findWidgetByName("ImageView");
    if (!widgetInterface)
        std::cerr << "Could not find a Qt Designer widget called \"ImageView\"" << std::endl;
}

bool ImageViewPlugin::probeOutputPort(rockdisplay::vizkitplugin::FieldDescription *fieldDesc, std::vector<std::string> &names)
{
    if(!widgetInterface)
        return false;
    return fieldDesc->getTypeName() == "/base/samples/frame/Frame" ||
           fieldDesc->getTypeName() == "/base/samples/DistanceImage" ||
           fieldDesc->getTypeName() == "/base/samples/frame/FramePair";
}

bool ImageViewPlugin::probeInputPort(rockdisplay::vizkitplugin::FieldDescription *fieldDesc, std::vector<std::string> &names) {
    if(!widgetInterface)
        return false;
    return fieldDesc->getTypeName() == "/base/samples/frame/Frame" ||
           fieldDesc->getTypeName() == "/base/samples/DistanceImage" ||
           fieldDesc->getTypeName() == "/base/samples/frame/FramePair";
}

bool ImageViewPlugin::probeProperty(rockdisplay::vizkitplugin::FieldDescription *fieldDesc, std::vector<std::string> &names) {
    if(!widgetInterface)
        return false;
    return fieldDesc->getTypeName() == "/base/samples/frame/Frame" ||
           fieldDesc->getTypeName() == "/base/samples/DistanceImage" ||
           fieldDesc->getTypeName() == "/base/samples/frame/FramePair";
}

rockdisplay::vizkitplugin::Widget *ImageViewPlugin::createWidget()
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

    QMetaMethod found_frame_method;
    QMetaMethod found_distanceimage_method;
    for(int i = 0 ; i < metaPlugin->methodCount(); i++)
    {
        QMetaMethod method = metaPlugin->method(i);
        auto parameterList = method.parameterTypes();
        if(parameterList.size() != 1)
        {
            continue;
        }
        if(parameterList[0].toStdString() != "base::samples::frame::Frame" &&
            parameterList[0].toStdString() != "base::samples::DistanceImage"
        )
        {
            continue;
        }

        std::string signature = method.methodSignature().toStdString();
        std::string methodStr("setFrame");
        if (signature.size() < methodStr.size() || signature.substr(0, methodStr.size()) != methodStr)
        {
            continue;
        }
        if (parameterList[0].toStdString() == "base::samples::frame::Frame")
        {
            found_frame_method = method;
        }
        if (parameterList[0].toStdString() == "base::samples::DistanceImage")
        {
            found_distanceimage_method = method;
        }
    }

    ImageViewWidget *ivvh = new ImageViewWidget(imView, found_frame_method,
            found_distanceimage_method);

    return ivvh;
}

std::string ImageViewPlugin::getName()
{
    return "ImageView";
}

unsigned ImageViewPlugin::getFlags() {
    return Flags::SingleFieldOnly;
}


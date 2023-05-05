
#include "imageviewplugin.hpp"
#include "imageviewplugin_p.hpp"
#include <rock_widget_collection/RockWidgetCollection.h>
#include <base/samples/Frame.hpp>

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
 */

using namespace rock_display;

ImageViewWidget::ImageViewWidget(QWidget *widget, QMetaMethod frame_method, QMetaMethod distanceimage_method, QObject *parent)
: rockdisplay::vizkitplugin::Widget(parent), widget(widget),
frame_method(frame_method), distanceimage_method(distanceimage_method),
outputportfield(new ImageViewOutputPortField(widget, frame_method, distanceimage_method)),
inputportfield(new ImageViewInputPortField(widget, frame_method, distanceimage_method)),
propertyfield(new ImageViewPropertyField(widget, frame_method, distanceimage_method))
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

ImageViewOutputPortField::ImageViewOutputPortField(QWidget *widget, QMetaMethod frame_method, QMetaMethod distanceimage_method, QObject *parent)
: rockdisplay::vizkitplugin::Field(parent), widget(widget),
frame_method(frame_method), distanceimage_method(distanceimage_method)
{
}

void ImageViewOutputPortField::updateOutputPort(rockdisplay::vizkitplugin::ValueHandle const *data)
{
    QMetaMethod method;
    QGenericArgument val;

    if (data->getFieldDescription()->getTypeName() == "/base/samples/frame/Frame")
    {
        method = frame_method;
        val = QGenericArgument("void *", data->getRawPtr());
    }
    else if (data->getFieldDescription()->getTypeName() == "/base/samples/DistanceImage")
    {
        method = distanceimage_method;
        val = QGenericArgument("void *", data->getRawPtr());
    }
    else if (data->getFieldDescription()->getTypeName() == "/base/samples/frame/FramePair")
    {
        base::samples::frame::FramePair const *v =
            reinterpret_cast<base::samples::frame::FramePair  const *>(
                data->getRawPtr());
        bool useFirst = true;
        if (useFirst)
        {
            val = QGenericArgument("void *", &v->first);
        }
        else
        {
            val = QGenericArgument("void *", &v->second);
        }
    }
    else
    {
        return;
    }
    if (!val.data())
    {
        return;
    }

    method.invoke(widget, val);
}

ImageViewInputPortField::ImageViewInputPortField(QWidget *widget, QMetaMethod frame_method, QMetaMethod distanceimage_method, QObject *parent)
: rockdisplay::vizkitplugin::Field(parent), widget(widget),
frame_method(frame_method), distanceimage_method(distanceimage_method)
{
}

void ImageViewInputPortField::updateInputPort(rockdisplay::vizkitplugin::ValueHandle *data)
{
    QMetaMethod method;
    QGenericArgument val;

    if (data->getFieldDescription()->getTypeName() == "/base/samples/frame/Frame")
    {
        method = frame_method;
        val = QGenericArgument("void *", data->getRawPtr());
    }
    else if (data->getFieldDescription()->getTypeName() == "/base/samples/DistanceImage")
    {
        method = distanceimage_method;
        val = QGenericArgument("void *", data->getRawPtr());
    }
    else if (data->getFieldDescription()->getTypeName() == "/base/samples/frame/FramePair")
    {
        base::samples::frame::FramePair const *v =
            reinterpret_cast<base::samples::frame::FramePair  const *>(
                data->getRawPtr());
        bool useFirst = true;
        if (useFirst)
        {
            val = QGenericArgument("void *", &v->first);
        }
        else
        {
            val = QGenericArgument("void *", &v->second);
        }
    }
    else
    {
        return;
    }
    if (!val.data())
    {
        return;
    }

    method.invoke(widget, val);
}

ImageViewPropertyField::ImageViewPropertyField(QWidget *widget, QMetaMethod frame_method, QMetaMethod distanceimage_method, QObject *parent)
: rockdisplay::vizkitplugin::Field(parent), widget(widget),
frame_method(frame_method), distanceimage_method(distanceimage_method)
{
}

void ImageViewPropertyField::updateProperty(rockdisplay::vizkitplugin::ValueHandle *data)
{
    QMetaMethod method;
    QGenericArgument val;

    if (data->getFieldDescription()->getTypeName() == "/base/samples/frame/Frame")
    {
        method = frame_method;
        val = QGenericArgument("void *", data->getRawPtr());
    }
    else if (data->getFieldDescription()->getTypeName() == "/base/samples/DistanceImage")
    {
        method = distanceimage_method;
        val = QGenericArgument("void *", data->getRawPtr());
    }
    else if (data->getFieldDescription()->getTypeName() == "/base/samples/frame/FramePair")
    {
        base::samples::frame::FramePair const *v =
            reinterpret_cast<base::samples::frame::FramePair  const *>(
                data->getRawPtr());
        bool useFirst = true;
        if (useFirst)
        {
            val = QGenericArgument("void *", &v->first);
        }
        else
        {
            val = QGenericArgument("void *", &v->second);
        }
    }
    else
    {
        return;
    }
    if (!val.data())
    {
        return;
    }

    method.invoke(widget, val);
}

bool ImageViewPlugin::probeOutputPort(rockdisplay::vizkitplugin::FieldDescription *fieldDesc, std::vector<std::string> &names)
{
    return fieldDesc->getTypeName() == "/base/samples/frame/Frame" ||
           fieldDesc->getTypeName() == "/base/samples/DistanceImage" ||
           fieldDesc->getTypeName() == "/base/samples/frame/FramePair";
}

bool ImageViewPlugin::probeInputPort(rockdisplay::vizkitplugin::FieldDescription *fieldDesc, std::vector<std::string> &names) {
    return fieldDesc->getTypeName() == "/base/samples/frame/Frame" ||
           fieldDesc->getTypeName() == "/base/samples/DistanceImage" ||
           fieldDesc->getTypeName() == "/base/samples/frame/FramePair";
}

bool ImageViewPlugin::probeProperty(rockdisplay::vizkitplugin::FieldDescription *fieldDesc, std::vector<std::string> &names) {
    return fieldDesc->getTypeName() == "/base/samples/frame/Frame" ||
           fieldDesc->getTypeName() == "/base/samples/DistanceImage" ||
           fieldDesc->getTypeName() == "/base/samples/frame/FramePair";
}

rockdisplay::vizkitplugin::Widget *ImageViewPlugin::createWidget()
{
    RockWidgetCollection collection;
    QList<QDesignerCustomWidgetInterface *> customWidgets = collection.customWidgets();

    QWidget *imView = nullptr;
    for (QDesignerCustomWidgetInterface *widgetInterface: customWidgets)
    {
        const std::string widgetName = widgetInterface->name().toStdString();

        if (widgetName == "ImageView")
        {
            imView = widgetInterface->createWidget(nullptr);
        }
    }

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
        if (parameterList[0].toStdString() != "base::samples::frame::Frame")
        {
            found_frame_method = method;
        }
        if (parameterList[0].toStdString() != "base::samples::DistanceImage")
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


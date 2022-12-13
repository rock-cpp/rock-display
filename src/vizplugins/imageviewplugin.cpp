
#include "imageviewplugin.hpp"
#include "imageviewplugin_p.hpp"
#include <rock_widget_collection/RockWidgetCollection.h>

using namespace rock_display;

ImageViewWidget::ImageViewWidget(QWidget *widget, QMetaMethod method, QObject *parent)
: rockdisplay::vizkitplugin::Widget(parent), widget(widget), method(method),
outputportfield(new ImageViewOutputPortField(widget, method)),
inputportfield(new ImageViewInputPortField(widget, method)),
propertyfield(new ImageViewPropertyField(widget, method))
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

ImageViewOutputPortField::ImageViewOutputPortField(QWidget *widget, QMetaMethod method, QObject *parent)
: rockdisplay::vizkitplugin::Field(parent), widget(widget), method(method)
{
}

void ImageViewOutputPortField::updateOutputPort(rockdisplay::vizkitplugin::ValueHandle const *data)
{
    QGenericArgument val("void *", data->getRawPtr());
    if (!val.data())
    {
        return;
    }

    method.invoke(widget, val);
}

ImageViewInputPortField::ImageViewInputPortField(QWidget *widget, QMetaMethod method, QObject *parent)
: rockdisplay::vizkitplugin::Field(parent), widget(widget), method(method)
{
}

void ImageViewInputPortField::updateInputPort(rockdisplay::vizkitplugin::ValueHandle *data)
{
    QGenericArgument val("void *", data->getRawPtr());
    if (!val.data())
    {
        return;
    }

    method.invoke(widget, val);
}

ImageViewPropertyField::ImageViewPropertyField(QWidget *widget, QMetaMethod method, QObject *parent)
: rockdisplay::vizkitplugin::Field(parent), widget(widget), method(method)
{
}

void ImageViewPropertyField::updateProperty(rockdisplay::vizkitplugin::ValueHandle *data)
{
    QGenericArgument val("void *", data->getRawPtr());
    if (!val.data())
    {
        return;
    }

    method.invoke(widget, val);
}

bool ImageViewPlugin::probeOutputPort(rockdisplay::vizkitplugin::FieldDescription *fieldDesc, std::vector<std::string> &names)
{
    return fieldDesc->getTypeName() == "/base/samples/frame/Frame";
}

bool ImageViewPlugin::probeInputPort(rockdisplay::vizkitplugin::FieldDescription *fieldDesc, std::vector<std::string> &names) {
    return fieldDesc->getTypeName() == "/base/samples/frame/Frame";
}

bool ImageViewPlugin::probeProperty(rockdisplay::vizkitplugin::FieldDescription *fieldDesc, std::vector<std::string> &names) {
    return fieldDesc->getTypeName() == "/base/samples/frame/Frame";
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
        std::string methodStr("setFrame");
        if (signature.size() > methodStr.size() && signature.substr(0, methodStr.size()) == methodStr)
        {
            found_method = method;
        }
    }

    ImageViewWidget *ivvh = new ImageViewWidget(imView, found_method);

    return ivvh;
}

std::string ImageViewPlugin::getName()
{
    return "ImageView";
}

unsigned ImageViewPlugin::getFlags() {
    return Flags::SingleFieldOnly;
}


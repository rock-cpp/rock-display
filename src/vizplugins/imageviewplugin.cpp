
#include "imageviewplugin.hpp"
#include <rock_widget_collection/RockWidgetCollection.h>
#include <typelib/typemodel.hh>

class ImageViewVizHandle : public VizHandle
{
public:
    QWidget *widget;
    QMetaMethod method;
    virtual void updateVisualizer(void const *data, RTT::base::DataSourceBase::shared_ptr base_sample) override;
    virtual QObject *getVizkit3dPluginObject() override { return nullptr; }
    virtual QWidget *getStandaloneWidget() override { return widget; }
};

ImageViewPluginHandle::ImageViewPluginHandle()
: PluginHandle("ImageView")
{
    typeName = "/base/samples/frame/Frame";
}

bool ImageViewPluginHandle::probe(Typelib::Type const &type, const Typelib::Registry* registry) const
{
    return type.getName() == typeName;
}

VizHandle *ImageViewPluginHandle::createViz() const
{
    RockWidgetCollection collection;
    QList<QDesignerCustomWidgetInterface *> customWidgets = collection.customWidgets();

    QWidget *imView = nullptr;
    for (QDesignerCustomWidgetInterface *widgetInterface: customWidgets)
    {
        const std::string widgetName = widgetInterface->name().toStdString();

        if (widgetName == pluginName)
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

    ImageViewVizHandle *ivvh = new ImageViewVizHandle;


    ivvh->method = found_method;
    ivvh->widget = imView;
    return ivvh;
}

void ImageViewVizHandle::updateVisualizer(void const *data, RTT::base::DataSourceBase::shared_ptr base_sample)
{
    QGenericArgument val("void *", data);
    if (!val.data())
    {
        return;
    }

    method.invoke(widget, val);
}




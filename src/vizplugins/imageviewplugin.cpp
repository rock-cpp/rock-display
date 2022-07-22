
#include "imageviewplugin.hpp"
#include "imageviewplugin_p.hpp"
#include <rock_widget_collection/RockWidgetCollection.h>
#include <typelib/typemodel.hh>

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

    imView->installEventFilter(ivvh);

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

bool ImageViewVizHandle::eventFilter(QObject *obj, QEvent *event) {
    if (event->type() == QEvent::Close) {
        emit closing(this);
    }
    return false;
}


/*
 * at the end, this should be no more complicated than the ruby part:
 *
 *
 *
 *
 * #prepares the c++ qt widget for the use in ruby with widget_grid
 *
 * Vizkit::UiLoader::extend_cplusplus_widget_class "ImageView" do
 *
 *   def default_options()
 *       options = Hash.new
 *       options[:time_overlay] = true
 *       options[:display_first] = true
 *       options
 *   end
 *
 *   def init
 *       if !defined? @init
 *           @options ||= default_options
 *           #connect(SIGNAL("activityChanged(bool)"),self,:setActive)
 *           @init = true
 *           @fallback = false
 *       end
 *   end
 *
 *   def display2(frame_pair,port_name)
 *       init
 *       frame = @options[:display_first] == true ? frame_pair.first : frame_pair.second
 *       display(frame,port_name)
 *   end
 *
 *   #display is called each time new data are available on the orocos output port
 *   #this functions translates the orocos data struct to the widget specific format
 *   def display(frame,port_name="")
 *       init
 *
 *       if @options[:time_overlay]
 *           if frame.time.instance_of?(Time)
 *               time = frame.time
 *           else
 *               time = Time.at(frame.time.seconds,frame.time.microseconds)
 *           end
 *           addTextWrapper(time.strftime("%F %H:%M:%S"), :bottomright, Qt::Color.new(Qt::black), false)
 *       end
 *
 *       if @fallback
 *           setRawImage(frame.frame_mode.to_s,frame.pixel_size,frame.size.width,frame.size.height,frame.image.to_byte_array[8..-1],frame.image.size)
 *       else
 *           @typelib_adapter ||= Vizkit::TypelibQtAdapter.new(self)
 *           if !@typelib_adapter.call_qt_method("setFrame",frame)
 *               Vizkit.warn "Cannot reach method setFrame."
 *               Vizkit.warn "This happens if an old log file is replayed and the type has changed."
 *               Vizkit.warn "Call rock-convert to update the logfile."
 *               Vizkit.warn "Falling back to use raw access."
 *               @fallback = true
 *               display(frame,port_name)
 *           end
 *       end
 *       update2
 *   end
 *
 *   def addTextWrapper(text, location, color, persistent)
 *       locationMap = {:topleft => 0,
 *           :topright => 1,
 *           :bottomleft => 2,
 *           :bottomright => 3}
 *       addText(text, locationMap[location], color, persistent)
 *   end
 *
 * end
 *
 * Vizkit::UiLoader.register_default_widget_for("ImageView","/base/samples/DistanceImage",:display)
 * Vizkit::UiLoader.register_default_widget_for("ImageView","/base/samples/frame/Frame",:display)
 * Vizkit::UiLoader.register_default_widget_for("ImageView","/base/samples/frame/FramePair",:display2)
 */


#include "plot2dplugin.hpp"
#include "plot2dplugin_p.hpp"
#include <base/samples/SonarBeam.hpp>
#include <base/samples/LaserScan.hpp>
#include <iostream>
#include "designer/pluginmanager_p.h"
#include <QWidget>
#include <QtUiPlugin/customwidget.h>
#include <QPen>
#include <typelib/typemodel.hh>
#include <QMouseEvent>
#include <QMenu>
#include <QFileDialog>
#include <QColorDialog>

/*
 * Plot2D
 *   is a data sink
 *   inputs any Typelib::NumericType using update
 *   inputs /base/Quaterniond using update_orientation
 *   inputs /base/samples/SonarBeam using update_sonar_beam
 *   inputs /base/samples/LaserScan using update_laser_scan
 *   inputs /base/Angle using update_angle
 *   inputs /base/Vector3d using update_vector3d
 *   inputs /base/VectorXd using update_vectorXd
 *   inputs /base/Time using update_time
 *   inputs /std/vector</uint8_t> using update_vector
 *   inputs /std/vector</uint16_t> using update_vector
 *   inputs /std/vector</uint32_t> using update_vector
 *   inputs /std/vector</uint64_t> using update_vector
 *   inputs /std/vector</int8_t> using update_vector
 *   inputs /std/vector</int16_t> using update_vector
 *   inputs /std/vector</int32_t> using update_vector
 *   inputs /std/vector</int64_t> using update_vector
 *   inputs /std/vector</float> using update_vector
 *   inputs /std/vector</double> using update_vector
 *   conversion update_angle
 *     update sample.rad
 *   conversion update_laser_scan
 *     update_vector sample.ranges
 *   conversion update_sonar_beam
 *     update_vector sample.beam
 *   conversion update_vectorXd
 *     sample.size() == 1?update sample[0]:foreach sample:s update s
 *     names: name[0]...name[x]
 *   conversion update vector3d
 *     update sample[0], name *_x
 *     update sample[1], name *_y
 *     update sample[2], name *_z
 *   conversion update_orientation:
 *     update (sample.yaw)   *(180.00/Math::PI), name *_yaw
 *     update (sample.pitch) *(180.00/Math::PI), name *_pitch
 *     update (sample.roll)  *(180.00/Math::PI), name *_roll
 *   conversion update_vector // replaces the complete series
 *     for each entry in vector graph.addData(index,value)
 *   conversion update
 *     graph.addData(time, value)
 *   has complicated ui in ruby
 *
 *
 *
 *
 *
 *
 * Vizkit::UiLoader::extend_cplusplus_widget_class "Plot2d" do
 *     attr_accessor :options
 *
 *     def default_options()
 *         options = Hash.new
 *         options[:auto_scrolling] = true     # auto scrolling for a specific axis is true
 *         options[:auto_scrolling_y] = false  # if one of auto_scrolling or auto_scrolling_<axis>
 *         options[:auto_scrolling_x] = false  # is true
 *         options[:time_window] = 30              #window size during auto scrolling
 *         options[:cached_time_window] = 60        #total cached window size
 *         options[:pre_time_window] = 5
 *         options[:xaxis_window] = 5
 *         options[:pre_xaxis_window] = 5
 *         options[:yaxis_window] = 5
 *         options[:pre_yaxis_window] = 5
 *         options[:max_points] = 50000
 *
 *         options[:colors] = [Qt::red, Qt::green, Qt::blue, Qt::cyan, Qt::magenta, Qt::yellow, Qt::gray]
 *         options[:reuse] = true
 *         options[:use_y_axis2] = false
 *         options[:plot_style] = :Line  #:Dot
 *         options[:multi_use_menu] = true
 *         options[:update_period] = 0.25   # repaint periode if new data are available
 *                                          # this prevents repainting for each new sample
 *         options[:plot_timestamps] = true
 *         options[:is_time_plot] = false
 *         return options
 *     end
 *
 *     def time
 *         time = if @log_replay
 *                    @log_replay.time
 *                else
 *                    Time.now
 *                end
 *         @time ||= time
 *         time
 *     end
 *
 *     def setXTitle(value)
 *         getXAxis.setLabel(value.to_s)
 *     end
 *
 *     def setYTitle(value)
 *         getYAxis.setLabel(value.to_s)
 *     end
 *
 *     def update_zoom_range_flag(flag, use_2nd_axis)
 *         y_axis = Hash[true => 1, false => 0][use_2nd_axis]
 *         setZoomAble flag, y_axis
 *         setRangeAble flag, y_axis
 *     end
 *
 *     def initialize_vizkit_extension
 *         @options = default_options
 *         @graphs = Hash.new
 *         @time = nil
 *         @timer = Qt::Timer.new
 *         @needs_update = false
 *         @timer.connect(SIGNAL"timeout()") do
 *             replot if @needs_update
 *             @needs_update = false
 *         end
 *         @timer.start(1000*@options[:update_period])
 *         @color_next_idx = 0
 *
 *         getLegend.setVisible(true)
 *         getXAxis.setLabel("Time in sec")
 *         setTitle("Rock-Plot2d")
 *
 *
 *         self.connect(SIGNAL('mousePressOnPlotArea(QMouseEvent*)')) do |event|
 *             if event.button() == Qt::RightButton
 *                 #show pop up menue
 *                 menu = Qt::Menu.new(self)
 *                 action_scrolling = Qt::Action.new("AutoScrolling", self)
 *                 action_scrolling.checkable = true
 *                 action_scrolling.checked = @options[:auto_scrolling]
 *                 menu.add_action(action_scrolling)
 *                 action_clear = Qt::Action.new("Clear", self)
 *                 menu.add_action(action_clear)
 *                 if @options[:multi_use_menu]
 *                     action_reuse = Qt::Action.new("Reuse Widget", self)
 *                     action_reuse.checkable = true
 *                     action_reuse.checked = @options[:reuse]
 *                     menu.add_action(action_reuse)
 *                     action_use_y2 = Qt::Action.new("Use 2. Y-Axis", self)
 *                     action_use_y2.checkable = true
 *                     action_use_y2.checked = @options[:use_y_axis2]
 *                     menu.add_action(action_use_y2)
 *                     action_plotdot = Qt::Action.new("'dot' style", self)
 *                     action_plotdot.checkable = true
 *                     action_plotdot.checked = @options[:plot_style] == :Dot
 *                     menu.add_action(action_plotdot)
 *                     action_plotline = Qt::Action.new("'line' style", self)
 *                     action_plotline.checkable = true
 *                     action_plotline.checked = @options[:plot_style] == :Line
 *                     menu.add_action(action_plotline)
 *                 end
 *                 menu.addSeparator
 *                 action_saving = Qt::Action.new("Save to File", self)
 *                 menu.add_action(action_saving)
 *                 if @options[:is_time_plot]
 *                     menu.addSeparator
 *                     action_timestamp = Qt::Action.new("Show timestamps", self)
 *                     action_timestamp.checkable = true
 *                     action_timestamp.checked = @options[:plot_timestamps]
 *                     menu.add_action(action_timestamp)
 *                     action_sample_period = Qt::Action.new("Show sample period", self)
 *                     action_sample_period.checkable = true
 *                     action_sample_period.checked = !@options[:plot_timestamps]
 *                     menu.add_action(action_sample_period)
 *                 end
 *
 *                 action = menu.exec(mapToGlobal(event.pos))
 *                 if(action == action_scrolling)
 *                     @options[:auto_scrolling] = !@options[:auto_scrolling]
 *                     update_zoom_range_flag(!@options[:auto_scrolling], @options[:use_y_axis2])
 *                 elsif(action == action_clear)
 *                     clearData()
 *                 elsif(action == action_reuse)
 *                     @options[:reuse] = !@options[:reuse]
 *                 elsif(action == action_use_y2)
 *                     update_zoom_range_flag(false, @options[:use_y_axis2])
 *                     @options[:use_y_axis2] = !@options[:use_y_axis2]
 *                     if @options[:use_y_axis2]
 *                         getYAxis2.setVisible(true)
 *                     end
 *                     update_zoom_range_flag(!@options[:auto_scrolling], @options[:use_y_axis2])
 *                  elsif(action == action_plotdot)
 *                     plot_style(:Dot)
 *                 elsif(action == action_plotline)
 *                     plot_style(:Line)
 *                 elsif action == action_saving
 *                     file_path = Qt::FileDialog::getSaveFileName(nil,"Save Plot to Pdf",File.expand_path("."),"Pdf (*.pdf)")
 *                     savePdf(file_path,false,0,0) if file_path
 *                 elsif ((action == action_timestamp) || (action == action_sample_period))
 *                     @options[:plot_timestamps] =! @options[:plot_timestamps]
 *                 end
 *             end
 *         end
 *
 *
 *         self.connect(SIGNAL('mousePressOnLegendItem(QMouseEvent*, QVariant)')) do |event, itemIdx|
 *             if event.button() == Qt::RightButton
 *                 #show pop up menue
 *                 menu = Qt::Menu.new(self)
 *                 action_remove = Qt::Action.new("remove graph", self)
 *                 menu.add_action(action_remove)
 *
 *                 action = menu.exec(mapToGlobal(event.pos))
 *
 *                 if(action == action_remove)
 *                     # note: we assume all graphs have a corresponding
 *                     # legend item with the same index (true for this widget)
 *                     graph = getGraph(itemIdx.to_i())
 *
 *                     unless graph == 0 || graph.nil?
 *
 *                         while true
 *                             cur_port = connection_manager().find_port_by_name(graph.name)
 *
 *                             if cur_port
 *                                 connection_manager().disconnect(cur_port,  keep_port: false)
 *                             else
 *                                 break
 *                             end
 *                         end
 *
 *                         @graphs.delete graph.name
 *                         removeGraph(itemIdx.to_i())
 *                         @needs_update = true
 *                     end
 *                 end
 *             end
 *         end
 *
 *     end
 *
 *     def graph_style(graph,style)
 *         if style == :Dot
 *             graph.setLineStyle(Qt::CPGraph::LSNone)
 *             graph.setScatterStyle(Qt::CPGraph::SSDot)
 *         else
 *             graph.setLineStyle(Qt::CPGraph::LSLine)
 *             graph.setScatterStyle(Qt::CPGraph::SSNone)
 *         end
 *         @needs_update = true
 *     end
 *
 *     def plot_style(style)
 *         if @options[:plot_style] != style
 *             @options[:plot_style] = style
 *             @graphs.each_value do |graph|
 *                 graph_style(graph,style)
 *             end
 *         end
 *     end
 *
 *     def config(value,options)
 *         @log_replay = if value.respond_to? :task
 *                           if value.task.respond_to? :log_replay
 *                               value.task.log_replay
 *                           end
 *                       end
 *         @options = options.merge(@options)
 *         if value.type_name == "/base/samples/SonarBeam"
 *             if !@graphs.empty?
 *                 puts "Cannot plot SonarBeam because plot is already used!"
 *                 return :do_not_connect
 *             else
 *                 @options[:multi_use_menu] = false
 *                 getXAxis.setLabel "Bin Number"
 *             end
 *         elsif value.type_name =~ /\/std\/vector</ || value.type_name == "/base/samples/LaserScan"
 *             if !@graphs.empty?
 *                 puts "Cannot plot std::vector because plot is already used!"
 *                 return :do_not_connect
 *             else
 *                 @options[:multi_use_menu] = false
 *                 getXAxis.setLabel "Index"
 *             end
 *         end
 *         subfield = Array(options[:subfield]).join(".")
 *         subfield = "." + subfield if !subfield.empty?
 *         graph2(value.full_name+subfield) if value.respond_to? :full_name
 *     end
 *
 *     def graph2(name)
 *         if(!@graphs.has_key?(name))
 *             axis = if @options[:use_y_axis2] then getYAxis2
 *                    else getYAxis
 *                    end
 *
 *             axis.setLabel(name.split(".").last)
 *             graph = addGraph(getXAxis(),axis)
 *             graph.setName(name)
 *             graph_style(graph,@options[:plot_style])
 *             graph.addToLegend
 *
 *             if color = @options[:colors][@color_next_idx]
 *                 graph.setPen(Qt::Pen.new(Qt::Brush.new(color),1))
 *             end
 *
 *             @color_next_idx = (@color_next_idx + 1) % @options[:colors].count
 *
 *             @graphs[name] = graph
 *         end
 *
 *         @graphs[name]
 *     end
 *
 *     def multi_value?
 *         @options[:reuse] && @options[:multi_use_menu]
 *     end
 *
 *     def rename_graph(old_name,new_name)
 *         graph = @graphs[old_name]
 *         if graph
 *             graph.setName(new_name)
 *             @graphs[new_name] = @graphs[old_name]
 *             @graphs.delete old_name
 *         end
 *     end
 *
 *     #diplay is called each time new data are available on the orocos output port
 *     #this functions translates the orocos data struct to the widget specific format
 *     def update(sample,name,time: self.time)
 *         graph = graph2(name)
 *         @time ||= time
 *         x = time-@time
 *         graph.removeDataBefore(x-@options[:cached_time_window])
 *         graph.addData(x,sample.to_f)
 *         if @options[:auto_scrolling] || @options[:auto_scrolling_x]
 *             getXAxis.setRange(x-@options[:time_window],x+@options[:pre_time_window])
 *             graph.rescaleValueAxis(true)
 *         end
 *         @needs_update = true
 *     end
 *
 *     def update_orientation(sample,name)
 *         rename_graph(name,name+"_yaw")
 *         update((sample.yaw)   *(180.00/Math::PI),name+"_yaw")
 *         update((sample.pitch) *(180.00/Math::PI),name+"_pitch")
 *         update((sample.roll)   *(180.00/Math::PI),name+"_roll")
 *     end
 *
 *     def update_vector3d(sample,name)
 *         rename_graph(name,name+"_x")
 *         update(sample[0],name+"_x")
 *         update(sample[1],name+"_y")
 *         update(sample[2],name+"_z")
 *     end
 *
 *     def update_vectorXd(sample,name)
 *         if (sample.size() == 1)
 *             update(sample[0], name)
 *         else
 *             rename_graph(name,name+"[0]")
 *             for i in (0..sample.size()-1)
 *                 update(sample[i], name+"["+i.to_s()+"]")
 *             end
 *         end
 *     end
 *
 *     def update_time(sample, name)
 *         # So that the time related options of the right click menu are not shown for other types
 *         @options[:is_time_plot] = true
 *         if @options[:plot_timestamps]
 *             update(sample.to_f, name)
 *         else
 *             update_time_diff(sample, name)
 *         end
 *     end
 *
 *     def update_time_diff(sample, name)
 *         # For each data source an entry in the dictionary is created
 *         if @previous_time == nil
 *             @previous_time = {}
 *         end
 *         if @previous_time[name] == nil
 *             @previous_time[name] = sample.to_f
 *         end
 *         difference = sample.to_f - @previous_time[name]
 *         @previous_time[name] = sample.to_f
 *         update(difference, name)
 *     end
 *
 *
 *     def set_x_axis_scale(start,stop)
 *         getXAxis.setRange(start,stop)
 *     end
 *
 *     def set_y_axis_scale(start,stop)
 *         getYAxis.setRange(start,stop)
 *     end
 *
 *     def update_custom(name,values_x,values_y)
 *         graph = graph2(name)
 *         graph.addData(values_x,values_y)
 *         if @options[:auto_scrolling] || @options[:auto_scrolling_x]
 *             getXAxis.setRange(values_x-@options[:xaxis_window],values_x+@options[:pre_xaxis_window])
 *             graph.rescaleValueAxis(true)
 *         end
 *         if @options[:auto_scrolling] || @options[:auto_scrolling_y]
 *             getYAxis.setRange(values_y-@options[:yaxis_window],values_y+@options[:pre_yaxis_window])
 *             graph.rescaleValueAxis(true)
 *         end
 *         @needs_update = true
 *     end
 *
 *     def update_vector(sample,name)
 *         if sample.size() > @options[:max_points]
 *             Vizkit.logger.warn "Cannot plot #{name}. Vector is too big"
 *             return
 *         end
 *         graph = graph2(name)
 *         graph.clearData
 *         sample.to_a.each_with_index do |value,index|
 *             graph.addData(index,value)
 *         end
 *         if @options[:auto_scrolling] || @options[:auto_scrolling_x]
 *             graph.rescaleKeyAxis(false)
 *         end
 *         if @options[:auto_scrolling] || @options[:auto_scrolling_y]
 *             graph.rescaleValueAxis(false)
 *         end
 *         @needs_update = true
 *     end
 *
 *     def update_sonar_beam(sample,name)
 *         update_vector sample.beam,name
 *     end
 *     def update_laser_scan(sample,name)
 *         update_vector sample.ranges,name
 *     end
 *     def update_angle(sample,name)
 *         update sample.rad,name
 *     end
 * end
 * vector_types = ["/std/vector</uint8_t>","/std/vector</uint16_t>","/std/vector</uint32_t>",
 *                 "/std/vector</uint64_t>","/std/vector</int8_t>","/std/vector</int16_t>",
 *                 "/std/vector</int32_t>","/std/vector</int64_t>","/std/vector</float>",
 *                 "/std/vector</double>"]
 * Vizkit::UiLoader.register_widget_for("Plot2d","Typelib::NumericType",:update)
 * Vizkit::UiLoader.register_widget_for("Plot2d","/base/Quaterniond",:update_orientation)
 * Vizkit::UiLoader.register_widget_for("Plot2d","/base/samples/SonarBeam",:update_sonar_beam)
 * Vizkit::UiLoader.register_widget_for("Plot2d","/base/samples/LaserScan",:update_laser_scan)
 * Vizkit::UiLoader.register_widget_for("Plot2d",vector_types,:update_vector)
 * Vizkit::UiLoader.register_widget_for("Plot2d","/base/Angle",:update_angle)
 * Vizkit::UiLoader.register_widget_for("Plot2d","/base/Vector3d",:update_vector3d)
 * Vizkit::UiLoader.register_widget_for("Plot2d","/base/VectorXd",:update_vectorXd)
 * Vizkit::UiLoader.register_widget_for("Plot2d","/base/Time",:update_time)
 *
 *
 *
 *
 *
 */

using namespace rock_display;

class QCPAxis;

Plot2dWidget::Options::Options() {
    auto_scrolling = true;// # auto scrolling for a specific axis is true
    auto_scrolling_y = false; // # if one of auto_scrolling or auto_scrolling_<axis>
    auto_scrolling_x = false; // # is true
    time_window = 30;             // #window size during auto scrolling
    cached_time_window = 60;       // #total cached window size
    pre_time_window = 5;
    xaxis_window = 5;
    pre_xaxis_window = 5;
    yaxis_window = 5;
    pre_yaxis_window = 5;
    max_points = 50000;

    colors = {Qt::red, Qt::green, Qt::blue, Qt::cyan, Qt::magenta, Qt::yellow, Qt::gray};
    reuse = true;
    use_y_axis2 = false;
    plot_style = Line; // #Dot;
    multi_use_menu = true;
    update_period = 0.25;  // # repaint periode if new data are available
                           // # this prevents repainting for each new sample
    plot_timestamps = true;
    is_time_plot = false;
}

Plot2dWidget::Options::~Options() {
    //for destroying the QVector<QColor> at a place where QColor is known
}

Plot2dWidget::Plot2dWidget(QWidget *widget, QMetaMethod frame_method, QMetaMethod distanceimage_method, QObject *parent)
: rockdisplay::vizkitplugin::Widget(parent), widget(widget),
frame_method(frame_method), distanceimage_method(distanceimage_method),
outputportfield(new Plot2dOutputPortField(this)),
inputportfield(new Plot2dInputPortField(this)),
propertyfield(new Plot2dPropertyField(this))
{
    options = Options();
    timer = new QTimer();
    needs_update = false;
    connect(timer, &QTimer::timeout,
            this, [this]()
    {
        if(needs_update)
            QMetaObject::invokeMethod(this->widget, "replot");
    });
    timer->start(1000* options.update_period);
    color_next_idx = 0;
    y1_needs_shrink = true;
    y2_needs_shrink = true;
    QObject *legend;
    QMetaObject::invokeMethod(
        widget, "getLegend", Q_RETURN_ARG(QObject *, legend));
    QMetaObject::invokeMethod(
        legend, "setVisible", Q_ARG(bool, true));
    QObject *xaxis;
    QMetaObject::invokeMethod(
        widget, "getXAxis", Q_RETURN_ARG(QObject *, xaxis));
    QMetaObject::invokeMethod(
        xaxis, "setLabel", Q_ARG(QString, QString("Time in sec")));
    QMetaObject::invokeMethod(
        widget, "setTitle", Q_ARG(QString, QString("Rock-Plot2d")));

    connect(widget, SIGNAL(mousePressOnPlotArea(QMouseEvent*)),
            this, SLOT(mousePressOnPlotArea(QMouseEvent*)));
    connect(widget, SIGNAL(mousePressOnLegendItem(QMouseEvent*, QVariant)),
            this, SLOT(mousePressOnLegendItem(QMouseEvent*, QVariant)));
}

void Plot2dWidget::mousePressOnPlotArea(QMouseEvent* event) {
    if (event->button() == Qt::RightButton)
    {
        // #show pop up menue
        QMenu *menu = new QMenu(widget);
        QAction *action_scrolling = new QAction("AutoScrolling", menu);
        action_scrolling->setCheckable(true);
        action_scrolling->setChecked(options.auto_scrolling);
        menu->addAction(action_scrolling);
        QAction *action_rescale = new QAction("Rescale", menu);
        menu->addAction(action_rescale);
        QAction *action_clear = new QAction("Clear", menu);
        menu->addAction(action_clear);
        QAction *action_reuse = nullptr;
        QAction *action_use_y2 = nullptr;
        QAction *action_plotdot = nullptr;
        QAction *action_plotline = nullptr;
        if (options.multi_use_menu)
        {
            action_reuse = new QAction("Reuse Widget", menu);
            action_reuse->setCheckable(true);
            action_reuse->setChecked(options.reuse);
            menu->addAction(action_reuse);
            action_use_y2 = new QAction("Use 2nd Y-Axis", menu);
            action_use_y2->setCheckable(true);
            action_use_y2->setChecked(options.use_y_axis2);
            menu->addAction(action_use_y2);
            QActionGroup *plotstyle = new QActionGroup(menu);
            plotstyle->setExclusive(true);
            action_plotdot = new QAction("'dot' style", menu);
            action_plotdot->setCheckable(true);
            action_plotdot->setChecked(options.plot_style == Dot);
            action_plotdot->setActionGroup(plotstyle);
            menu->addAction(action_plotdot);
            action_plotline = new QAction("'line' style", menu);
            action_plotline->setCheckable(true);
            action_plotline->setChecked(options.plot_style == Line);
            action_plotline->setActionGroup(plotstyle);
            menu->addAction(action_plotline);
        }
        menu->addSeparator();
        QAction *action_saving = new QAction("Save to File...", menu);
        menu->addAction(action_saving);
        QAction *action_timestamp = nullptr;
        QAction *action_sample_period = nullptr;
        if(options.is_time_plot) {
            QActionGroup *time_format = new QActionGroup(menu);
            time_format->setExclusive(true);
            menu->addSeparator();
            action_timestamp = new QAction("Show timestamps", menu);
            action_timestamp->setCheckable(true);
            action_timestamp->setChecked(options.plot_timestamps);
            action_timestamp->setActionGroup(time_format);
            menu->addAction(action_timestamp);
            action_sample_period = new QAction("Show sample period", menu);
            action_sample_period->setCheckable(true);
            action_sample_period->setChecked(!options.plot_timestamps);
            action_sample_period->setActionGroup(time_format);
            menu->addAction(action_sample_period);
        }
        QAction *action = menu->exec(widget->mapToGlobal(event->pos()));
        if (action == nullptr)
        {
        }
        else if (action == action_scrolling)
        {
            options.auto_scrolling = !options.auto_scrolling;
            update_zoom_range_flag(!options.auto_scrolling,
                                   options.use_y_axis2);
        }
        else if (action == action_rescale)
        {
            rescale_yaxis();
        }
        else if (action == action_clear)
        {
            QMetaObject::invokeMethod(widget, "clearData");
            for (auto &g : graphs)
            {
                g.second.num_elements = 0;
            }
            y1_needs_shrink = true;
            y2_needs_shrink = true;
        }
        else if (action == action_reuse)
        {
            options.reuse = !options.reuse;
        }
        else if (action == action_use_y2)
        {
            update_zoom_range_flag(false, options.use_y_axis2);
            options.use_y_axis2 = !options.use_y_axis2;
            if (options.use_y_axis2)
            {
                QObject *yaxis2;
                QMetaObject::invokeMethod(widget, "getYAxis2",
                                          Q_RETURN_ARG(QObject *, yaxis2));
                QMetaObject::invokeMethod(yaxis2, "setVisible",
                                          Q_ARG(bool, true));
            }
            update_zoom_range_flag(!options.auto_scrolling,
                                   options.use_y_axis2);
        }
        else if (action == action_plotdot)
        {
            plot_style(Dot);
        }
        else if (action == action_plotline)
        {
            plot_style(Line);
        }
        else if (action == action_saving)
        {
            QString file_path = QFileDialog::getSaveFileName(
                                    nullptr, "Save Plot to Pdf",
                                    QDir::currentPath(), "Pdf (*.pdf)");
            QMetaObject::invokeMethod(widget, "savePDF",
                                      Q_ARG(QString, file_path),
                                      Q_ARG(bool, false),
                                      Q_ARG(int, 0), Q_ARG(int, 0));
        }
        else if (action == action_timestamp)
        {
            options.plot_timestamps = true;
            //this leaves the old data in the graph, so no point in rescaling
            //here until the user does a clearData.
        }
        else if (action == action_sample_period)
        {
            options.plot_timestamps = false;
            //this leaves the old data in the graph, so no point in rescaling
            //here until the user does a clearData.
        }
    }
}

void Plot2dWidget::mousePressOnLegendItem(QMouseEvent *event, QVariant itemIdx)
{
    if (event->button() == Qt::RightButton)
    {
        QObject *graph;
        QMetaObject::invokeMethod(widget, "getGraph",
                                  Q_RETURN_ARG(QObject *, graph),
                                  Q_ARG(int, itemIdx.toInt()));
        if (graph)
        {
            QString name;
            QMetaObject::invokeMethod(graph, "name", Q_RETURN_ARG(QString, name));
            auto &graphinfo = graphs[name.toStdString()];

            // #show pop up menue
            QMenu *menu = new QMenu(widget);
            QAction *action_clear = new QAction("Clear", menu);
            menu->addAction(action_clear);
            QAction *action_use_y2 = nullptr;
            if (options.multi_use_menu)
            {
                action_use_y2 = new QAction("Use 2nd Y-Axis", menu);
                action_use_y2->setCheckable(true);
                action_use_y2->setChecked(graphinfo.use_y_axis2);
                menu->addAction(action_use_y2);
            }
            QActionGroup *plotstyle = new QActionGroup(menu);
            plotstyle->setExclusive(true);
            QAction *action_plotdot = new QAction("'dot' style", menu);
            action_plotdot->setCheckable(true);
            action_plotdot->setChecked(graphinfo.style == Dot);
            action_plotdot->setActionGroup(plotstyle);
            menu->addAction(action_plotdot);
            QAction *action_plotline = new QAction("'line' style", menu);
            action_plotline->setCheckable(true);
            action_plotline->setChecked(graphinfo.style == Line);
            action_plotline->setActionGroup(plotstyle);
            menu->addAction(action_plotline);
            QAction *action_color = new QAction("Set color...", menu);
            menu->addAction(action_color);
            QAction *action_remove = new QAction("remove graph", menu);
            menu->addAction(action_remove);

            QAction *action = menu->exec(widget->mapToGlobal(event->pos()));
            if (action == nullptr)
            {
            }
            else if (action == action_clear)
            {
                QMetaObject::invokeMethod(graph, "clearData");
                graphinfo.num_elements = 0;
                if(graphinfo.use_y_axis2)
                    y2_needs_shrink = true;
                else
                    y1_needs_shrink = true;
                //this works when there are other graphs using the same
                //axis. no rescaling happens when asking empty graphs.
                rescale_yaxis(!graphinfo.use_y_axis2, graphinfo.use_y_axis2);
            }
            else if (action == action_use_y2)
            {
                graphinfo.use_y_axis2 = !graphinfo.use_y_axis2;
                QObject *yaxis;
                if (graphinfo.use_y_axis2)
                {
                    QMetaObject::invokeMethod(
                        widget, "getYAxis2", Q_RETURN_ARG(QObject *, yaxis));
                    //also making sure the second axis is visible
                    QMetaObject::invokeMethod(yaxis, "setVisible",
                                              Q_ARG(bool, true));
                }
                else
                {
                    QMetaObject::invokeMethod(
                        widget, "getYAxis", Q_RETURN_ARG(QObject *, yaxis));
                }
                QMetaObject::invokeMethod(graph, "setValueAxis",
                                          Q_ARG(QCPAxis *, reinterpret_cast<QCPAxis *>(yaxis)));

                //and then, rescale them all because during add, some things may have
                //become invisibe
                rescale_yaxis();

                needs_update = true;
            }
            else if (action == action_plotdot)
            {
                graphinfo.style = Dot;
                graph_style(graph, graphinfo.style);
            }
            else if (action == action_plotline)
            {
                graphinfo.style = Line;
                graph_style(graph, graphinfo.style);
            }
            else if (action == action_color)
            {
                QColor color = QColorDialog::getColor(graphinfo.color);
                if (color.isValid())
                {
                    graphinfo.color = color;
                    QMetaObject::invokeMethod(graph, "setPen",
                                              Q_ARG(QPen, QPen(color)));
                }
            }
            else if (action == action_remove)
            {
                // # note: we assume all graphs have a corresponding
                // # legend item with the same index (true for this widget)
                while (true)
                {
                    //TODO we would probably try to find the
                    // rockdisplay::vizkitplugin::Field we created for the
                    // graph and tell that one to go away.
                    //
                    // cur_port = connection_manager().find_port_by_name(graph.name)
                    //
                    // if cur_port
                    //     connection_manager().disconnect(cur_port,  keep_port: false)
                    // else
                    break;
                    // end
                }
                graphs.erase(name.toStdString());
                QMetaObject::invokeMethod(widget, "removeGraph",
                                          Q_ARG(int, itemIdx.toInt()));
                needs_update = true;
            }
        }
    }
}

void Plot2dWidget::rescale_yaxis(bool y1_axis, bool y2_axis) {
    bool first_y1 = true;
    bool first_y2 = true;
    for (auto &g : graphs)
    {
        if (g.second.num_elements < 2)
            continue;
        //rescaleValueAxis has no effect if there are too few elements
        if (g.second.use_y_axis2)
        {
            if(y2_axis) {
                QMetaObject::invokeMethod(g.second.graph, "rescaleValueAxis",
                                          Q_ARG(bool, !first_y2));
                first_y2 = false;
                y2_needs_shrink = false;
            }
        }
        else {
            if (y1_axis)
            {
                QMetaObject::invokeMethod(g.second.graph, "rescaleValueAxis",
                                          Q_ARG(bool, !first_y1));
                first_y1 = false;
                y1_needs_shrink = false;
            }
        }
    }
}

void Plot2dWidget::update_zoom_range_flag(bool flag, bool use_2nd_axis)
{
    QMetaObject::invokeMethod(widget, "setZoomAble",
        Q_ARG(bool, flag), Q_ARG(int, use_2nd_axis?1:0));
    QMetaObject::invokeMethod(widget, "setRangeAble",
        Q_ARG(bool, flag), Q_ARG(int, use_2nd_axis?1:0));
}

bool Plot2dWidget::config(const rockdisplay::vizkitplugin::FieldDescription *type,
                std::string const &subpluginname) {
    std::string tn = type->getTypeName();
    if (tn == "/base/samples/SonarBeam")
    {
        if (!graphs.empty())
        {
            std::cerr <<
                      "Cannot plot SonarBeam because plot is already used!" <<
                      std::endl;
            return false;
        }
        else
        {
            options.multi_use_menu = false;
        }
    }
    else if (tn.rfind("/std/vector<", 0) == 0 ||
             tn == "/base/samples/LaserScan")
    {
        if (!graphs.empty())
        {
            std::cerr <<
                      "Cannot plot std::vector because plot is already used!" <<
                      std::endl;
            return false;
        }
        else
        {
            options.multi_use_menu = false;
            QObject *xaxis;
            QMetaObject::invokeMethod(widget, "getXAxis",
                                      Q_RETURN_ARG(QObject*,xaxis));
            QMetaObject::invokeMethod(xaxis, "setLabel",
                                      Q_ARG(QString, QString("Index")));
        }
    }
    std::string name = type->getTaskName() + "::" +
            //the *PortName report empty strings if they are not of that kind
                   type->getInputPortName() +
                   type->getOutputPortName() +
                   type->getPropertyName() + "." +
                   type->getFieldName();
    graph2(name);
    return true;
}

QWidget *Plot2dWidget::getWidget()
{
    return widget;
}

rockdisplay::vizkitplugin::Field *Plot2dWidget::addOutputPortField( const rockdisplay::vizkitplugin::FieldDescription * type, std::string const &subpluginname )
{
    if(!config(type, subpluginname))
        return nullptr;
    return outputportfield;
}

rockdisplay::vizkitplugin::Field *Plot2dWidget::addInputPortField( const rockdisplay::vizkitplugin::FieldDescription * type, std::string const &subpluginname )
{
    if(!config(type, subpluginname))
        return nullptr;
    return inputportfield;
}

rockdisplay::vizkitplugin::Field *Plot2dWidget::addPropertyField( const rockdisplay::vizkitplugin::FieldDescription * type, std::string const &subpluginname )
{
    if(!config(type, subpluginname))
        return nullptr;
    return propertyfield;
}

base::Time Plot2dWidget::current_time() {
    base::Time time = base::Time::now();
    if (!time0_valid)
    {
        time0 = time;
        time0_valid = true;
    }
    return time;
}

void Plot2dWidget::rename_graph(std::string const &old_name, std::string const &new_name) {
    if(old_name == new_name)
        return;
    auto it = graphs.find(old_name);
    if(it != graphs.end()) {
        auto graph = *it;
        graphs.erase(it);
        graphs[new_name] = graph.second;
    }
}

void Plot2dWidget::graph_style(QObject *graph, GraphStyle style)
{
    if(style == Dot) {
        QMetaObject::invokeMethod(graph, "setLineStyle", Q_ARG(int, 0));//LSNone
        QMetaObject::invokeMethod(graph, "setScatterStyle", Q_ARG(int, 1));//SSDot
    } else {
        QMetaObject::invokeMethod(graph, "setLineStyle", Q_ARG(int, 1));//LSLine
        QMetaObject::invokeMethod(graph, "setScatterStyle", Q_ARG(int, 0));//SSNone
    }
    needs_update = true;
}

void Plot2dWidget::plot_style(GraphStyle style)
{
    if (options.plot_style != style)
    {
        options.plot_style = style;
        for (auto &g : graphs)
        {
            graph_style(g.second.graph, style);
        }
    }
}

QObject *Plot2dWidget::graph2(std::string const &name) {
    auto it = graphs.find(name);
    if (it == graphs.end())
    {
        QObject *yaxis;
        if (options.use_y_axis2)
        {
            QMetaObject::invokeMethod(
                widget, "getYAxis2", Q_RETURN_ARG(QObject *, yaxis));
        }
        else
        {
            QMetaObject::invokeMethod(
                widget, "getYAxis", Q_RETURN_ARG(QObject *, yaxis));
        }
        //TODO vizkit version does name.split(".").last
        QMetaObject::invokeMethod(
            yaxis, "setLabel", Q_ARG(QString, QString::fromStdString(name)));
        QObject *xaxis;
        QMetaObject::invokeMethod(widget, "getXAxis",
                                           Q_RETURN_ARG(QObject *, xaxis));

        QObject *graph;
        QMetaObject::invokeMethod(widget, "addGraph",
                                           Q_RETURN_ARG(QObject *, graph),
                                           Q_ARG(QObject *, xaxis),
                                           Q_ARG(QObject *, yaxis));

        QMetaObject::invokeMethod(
            graph, "setName", Q_ARG(QString, QString::fromStdString(name)));

        graph_style(graph, options.plot_style);
        QMetaObject::invokeMethod(graph, "addToLegend");

        QColor color = options.colors[color_next_idx];
        if(!options.colors.empty()) {
            QMetaObject::invokeMethod(graph, "setPen",
                Q_ARG(QPen, QPen(color)));
        }

        color_next_idx = (color_next_idx + 1) % options.colors.size();
        GraphInfo gi = {graph, options.use_y_axis2, color, options.plot_style, 0};
        graphs[name] = gi;
        return graph;
    } else {
        return it->second.graph;
    }

}

void Plot2dWidget::update(double sample, std::string const &name, base::Time const &time) {
    /* def update */
    // #display is called each time new data are available on the orocos output port
    // #this functions translates the orocos data struct to the widget specific format
    QObject *graph = graph2(name);
    GraphInfo &graphinfo = graphs[name];
    if (!time0_valid)
    {
        time0 = time;
        time0_valid = true;
    }
    double x = (time - time0).toSeconds();
    QMetaObject::invokeMethod(graph, "removeDataBefore",
                              Q_ARG(double, x - options.cached_time_window));
    QMetaObject::invokeMethod(graph, "addData",
                              Q_ARG(double, x),
                              Q_ARG(double, sample));
    graphinfo.num_elements++;//no need to try to account for removal here. num_elements is only relevant for very small numbers( < 3)
    if (options.auto_scrolling || options.auto_scrolling_x)
    {
        QObject *xaxis;
        QMetaObject::invokeMethod(widget, "getXAxis",
                                  Q_RETURN_ARG(QObject *, xaxis));

        QMetaObject::invokeMethod(xaxis, "setRange",
                                  Q_ARG(double, x - options.time_window),
                                  Q_ARG(double, x + options.time_window));

        if (graphinfo.num_elements >= 2)
        {
            //rescaleValueAxis does nothing if there is not enough data.
            if (graphinfo.use_y_axis2)
            {
                QMetaObject::invokeMethod(graph, "rescaleValueAxis",
                                          Q_ARG(bool, !y2_needs_shrink));
                y2_needs_shrink = false;
            }
            else
            {
                QMetaObject::invokeMethod(graph, "rescaleValueAxis",
                                          Q_ARG(bool, !y1_needs_shrink));
                y1_needs_shrink = false;
            }
        }
    }
    needs_update = true;
}

void Plot2dWidget::update_vector(std::vector<double> const &sample, std::string const &name)
{
    /* def update_vector */
    if (sample.size() > options.max_points)
    {
        std::cerr << "Cannot plot " << name << ". Vector is too big" << std::endl;
        return;
    }
    QObject *graph = graph2(name);
    GraphInfo &graphinfo = graphs[name];
    QMetaObject::invokeMethod(graph, "clearData");
    graphinfo.num_elements = 0;
    for (unsigned i = 0; i < sample.size(); i++)
    {
        QMetaObject::invokeMethod(graph, "addData",
                                          Q_ARG(double, i),
                                          Q_ARG(double, sample[i]));
    }
    graphinfo.num_elements = sample.size();
    if (options.auto_scrolling || options.auto_scrolling_x)
    {
        QMetaObject::invokeMethod(graph, "rescaleKeyAxis", Q_ARG(bool, false));
    }
    if (options.auto_scrolling || options.auto_scrolling_y)
    {
        QMetaObject::invokeMethod(graph, "rescaleValueAxis", Q_ARG(bool, false));
    }
    needs_update = true;
}

void Plot2dWidget::doUpdate(const rockdisplay::vizkitplugin::ValueHandle *data) {
    std::string tn = data->getFieldDescription()->getTypeName();
    std::string name = data->getFieldDescription()->getTaskName() + "::" +
            //the *PortName report empty strings if they are not of that kind
                   data->getFieldDescription()->getInputPortName() +
                   data->getFieldDescription()->getOutputPortName() +
                   data->getFieldDescription()->getPropertyName() + "." +
                   data->getFieldDescription()->getFieldName();
    if (data->getFieldDescription()->getType()->getCategory() ==
            Typelib::Type::Category::Numeric)
    {
        Typelib::Numeric const *num = static_cast<Typelib::Numeric const *>(data->getFieldDescription()->getType());
        double value;
        bool value_valid = false;
        switch (num->getNumericCategory())
        {
            case Typelib::Numeric::Float:
                if (num->getSize() == sizeof(float))
                {
                    value = *reinterpret_cast<float const *>(data->getRawPtr());
                    value_valid = true;
                }
                else if (num->getSize() == sizeof(double))
                {
                    value = *reinterpret_cast<double const *>(data->getRawPtr());
                    value_valid = true;
                }
                else if (num->getSize() == sizeof(long double))
                {
                    value = *reinterpret_cast<long double const *>(data->getRawPtr());
                    value_valid = true;
                }
                break;
            case Typelib::Numeric::SInt:
                if (num->getSize() == sizeof(int8_t))
                {
                    value = *reinterpret_cast<int8_t const *>(data->getRawPtr());
                    value_valid = true;
                }
                else if (num->getSize() == sizeof(int16_t))
                {
                    value = *reinterpret_cast<int16_t const *>(data->getRawPtr());
                    value_valid = true;
                }
                else if (num->getSize() == sizeof(int32_t))
                {
                    value = *reinterpret_cast<int32_t const *>(data->getRawPtr());
                    value_valid = true;
                }
                else if (num->getSize() == sizeof(int64_t))
                {
                    value = *reinterpret_cast<int64_t const *>(data->getRawPtr());
                    value_valid = true;
                }
                break;
            case Typelib::Numeric::UInt:
                if (num->getSize() == sizeof(uint8_t))
                {
                    value = *reinterpret_cast<uint8_t const *>(data->getRawPtr());
                    value_valid = true;
                }
                else if (num->getSize() == sizeof(uint16_t))
                {
                    value = *reinterpret_cast<uint16_t const *>(data->getRawPtr());
                    value_valid = true;
                }
                else if (num->getSize() == sizeof(uint32_t))
                {
                    value = *reinterpret_cast<uint32_t const *>(data->getRawPtr());
                    value_valid = true;
                }
                else if (num->getSize() == sizeof(uint64_t))
                {
                    value = *reinterpret_cast<uint64_t const *>(data->getRawPtr());
                    value_valid = true;
                }
                break;
            default:
                break;
        }

        if (value_valid)
        {
            update(value, name, current_time());
        }
    }
    else if (tn == "/base/Quaterniond")
    {
        /* def update_orientation */
        rename_graph(name, name+"_yaw");
        base::Quaterniond const *sample =
            reinterpret_cast<base::Quaterniond const *>(data->getRawPtr());
        //lifted from base/types ruby bindings
        float pitch;
        float roll;
        float yaw;
        const Eigen::Matrix3d m = sample->toRotationMatrix();
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
        update(yaw * (180.0/M_PI), name+"_yaw", current_time());
        update(pitch * (180.0/M_PI), name+"_pitch", current_time());
        update(roll * (180.0/M_PI), name+"_roll", current_time());
    }
    else if (tn == "/base/samples/SonarBeam")
    {
        /* def update_sonar_beam */
        base::samples::SonarBeam const *sample =
            reinterpret_cast<base::samples::SonarBeam const *>(data->getRawPtr());
        std::vector<double> vect(sample->beam.begin(), sample->beam.end());
        update_vector(vect, name);
    }
    else if (tn == "/base/samples/LaserScan")
    {
        /* def update_laser_scan */
        base::samples::LaserScan const *sample =
            reinterpret_cast<base::samples::LaserScan const *>(data->getRawPtr());
        std::vector<double> vect(sample->ranges.begin(), sample->ranges.end());
        update_vector(vect, name);
    }
    else if (tn == "/std/vector</uint8_t>") {
        std::vector<uint8_t> const *sample =
            reinterpret_cast<std::vector<uint8_t> const *>(data->getRawPtr());
        std::vector<double> vect(sample->begin(), sample->end());
        update_vector(vect, name);
    }
    else if (tn == "/std/vector</uint16_t>") {
        std::vector<uint16_t> const *sample =
            reinterpret_cast<std::vector<uint16_t> const *>(data->getRawPtr());
        std::vector<double> vect(sample->begin(), sample->end());
        update_vector(vect, name);
    }
    else if (tn == "/std/vector</uint32_t>") {
        std::vector<uint32_t> const *sample =
            reinterpret_cast<std::vector<uint32_t> const *>(data->getRawPtr());
        std::vector<double> vect(sample->begin(), sample->end());
        update_vector(vect, name);
    }
    else if (tn == "/std/vector</uint64_t>") {
        std::vector<uint64_t> const *sample =
            reinterpret_cast<std::vector<uint64_t> const *>(data->getRawPtr());
        std::vector<double> vect(sample->begin(), sample->end());
        update_vector(vect, name);
    }
    else if (tn == "/std/vector</int8_t>") {
        std::vector<int8_t> const *sample =
            reinterpret_cast<std::vector<int8_t> const *>(data->getRawPtr());
        std::vector<double> vect(sample->begin(), sample->end());
        update_vector(vect, name);
    }
    else if (tn == "/std/vector</int16_t>") {
        std::vector<int16_t> const *sample =
            reinterpret_cast<std::vector<int16_t> const *>(data->getRawPtr());
        std::vector<double> vect(sample->begin(), sample->end());
        update_vector(vect, name);
    }
    else if (tn == "/std/vector</int32_t>") {
        std::vector<int32_t> const *sample =
            reinterpret_cast<std::vector<int32_t> const *>(data->getRawPtr());
        std::vector<double> vect(sample->begin(), sample->end());
        update_vector(vect, name);
    }
    else if (tn == "/std/vector</int64_t>") {
        std::vector<int64_t> const *sample =
            reinterpret_cast<std::vector<int64_t> const *>(data->getRawPtr());
        std::vector<double> vect(sample->begin(), sample->end());
        update_vector(vect, name);
    }
    else if (tn == "/std/vector</float>") {
        std::vector<float> const *sample =
            reinterpret_cast<std::vector<float> const *>(data->getRawPtr());
        std::vector<double> vect(sample->begin(), sample->end());
        update_vector(vect, name);
    }
    else if (tn == "/std/vector</double>") {
        std::vector<double> const *sample =
            reinterpret_cast<std::vector<double> const *>(data->getRawPtr());
        update_vector(*sample, name);
    }
    else if (tn == "/base/Angle")
    {
        base::Angle const *sample =
            reinterpret_cast<base::Angle const *>(data->getRawPtr());
        update(sample->rad, name, current_time());
    }
    else if (tn == "/base/Vector3d")
    {
        base::Vector3d const *sample =
            reinterpret_cast<base::Vector3d const *>(data->getRawPtr());
        rename_graph(name, name + "_x");
        update((*sample)[0], name + "_x", current_time());
        update((*sample)[1], name + "_y", current_time());
        update((*sample)[2], name + "_z", current_time());
    }
    else if (tn == "/base/VectorXd")
    {
        base::VectorXd const *sample =
            reinterpret_cast<base::VectorXd const *>(data->getRawPtr());
        if (sample->size() == 1)
        {
            update((*sample)[0], name, current_time());
        }
        else
        {
            rename_graph(name, name + "[0]");
            for(unsigned i = 0; i < sample->size(); i++) {
                update((*sample)[i],
                       QString("%1[%2]").
                       arg(QString::fromStdString(name)).arg(i).toStdString(),
                       current_time());
            }
        }
    }
    else if (tn == "/base/Time")
    {
        base::Time const *sample =
            reinterpret_cast<base::Time const *>(data->getRawPtr());
        // # So that the time related options of the right click menu are not shown for other types
        options.is_time_plot = true;
        if (options.plot_timestamps)
        {
            update(sample->toSeconds(), name, current_time());
        }
        else
        {
            if(previous_time.find(name) == previous_time.end()) {
                previous_time[name] = *sample;
            }
            base::Time diff = *sample - previous_time[name];
            previous_time[name] = *sample;
            update(diff.toSeconds(), name, current_time());
        }
    }
}

Plot2dOutputPortField::Plot2dOutputPortField(Plot2dWidget *widget, QObject *parent)
: rockdisplay::vizkitplugin::Field(parent), widget(widget)
{
}

void Plot2dOutputPortField::updateOutputPort(rockdisplay::vizkitplugin::ValueHandle const *data)
{
    widget->doUpdate(data);
}

Plot2dInputPortField::Plot2dInputPortField(Plot2dWidget *widget, QObject *parent)
: rockdisplay::vizkitplugin::Field(parent), widget(widget)
{
}

void Plot2dInputPortField::updateInputPort(rockdisplay::vizkitplugin::ValueHandle *data)
{
    widget->doUpdate(data);
}

Plot2dPropertyField::Plot2dPropertyField(Plot2dWidget *widget, QObject *parent)
: rockdisplay::vizkitplugin::Field(parent), widget(widget)
{
}

void Plot2dPropertyField::updateProperty(rockdisplay::vizkitplugin::ValueHandle *data)
{
    widget->doUpdate(data);
}

Plot2dPlugin::Plot2dPlugin()
    : widgetInterface(nullptr)
{
    widgetInterface = rockdisplay::QDesignerPluginManager::getInstance()->
                      findWidgetByName("Plot2d");
    if (!widgetInterface)
        std::cerr << "Could not find a Qt Designer widget called \"Plot2d\"" << std::endl;
}

bool Plot2dPlugin::probeOutputPort(rockdisplay::vizkitplugin::FieldDescription *fieldDesc, std::vector<std::string> &names)
{
    if(!widgetInterface)
        return false;
    std::string tn = fieldDesc->getTypeName();
    return fieldDesc->getType()->getCategory() ==
           Typelib::Type::Category::Numeric || tn == "/base/Quaterniond" ||
           tn == "/base/samples/SonarBeam" || tn == "/base/samples/LaserScan" ||
           tn == "/std/vector</uint8_t>" || tn == "/std/vector</uint16_t>" ||
           tn == "/std/vector</uint32_t>" || tn == "/std/vector</uint64_t>" ||
           tn == "/std/vector</int8_t>" || tn == "/std/vector</int16_t>" ||
           tn == "/std/vector</int32_t>" || tn == "/std/vector</int64_t>" ||
           tn == "/std/vector</float>" || tn == "/std/vector</double>" ||
           tn == "/base/Angle" || tn == "/base/Vector3d" ||
           tn == "/base/VectorXd" || tn == "/base/Time";
}

bool Plot2dPlugin::probeInputPort(rockdisplay::vizkitplugin::FieldDescription *fieldDesc, std::vector<std::string> &names) {
    if(!widgetInterface)
        return false;
    std::string tn = fieldDesc->getTypeName();
    return fieldDesc->getType()->getCategory() ==
           Typelib::Type::Category::Numeric || tn == "/base/Quaterniond" ||
           tn == "/base/samples/SonarBeam" || tn == "/base/samples/LaserScan" ||
           tn == "/std/vector</uint8_t>" || tn == "/std/vector</uint16_t>" ||
           tn == "/std/vector</uint32_t>" || tn == "/std/vector</uint64_t>" ||
           tn == "/std/vector</int8_t>" || tn == "/std/vector</int16_t>" ||
           tn == "/std/vector</int32_t>" || tn == "/std/vector</int64_t>" ||
           tn == "/std/vector</float>" || tn == "/std/vector</double>" ||
           tn == "/base/Angle" || tn == "/base/Vector3d" ||
           tn == "/base/VectorXd" || tn == "/base/Time";
}

bool Plot2dPlugin::probeProperty(rockdisplay::vizkitplugin::FieldDescription *fieldDesc, std::vector<std::string> &names) {
    if(!widgetInterface)
        return false;
    std::string tn = fieldDesc->getTypeName();
    return fieldDesc->getType()->getCategory() ==
           Typelib::Type::Category::Numeric || tn == "/base/Quaterniond" ||
           tn == "/base/samples/SonarBeam" || tn == "/base/samples/LaserScan" ||
           tn == "/std/vector</uint8_t>" || tn == "/std/vector</uint16_t>" ||
           tn == "/std/vector</uint32_t>" || tn == "/std/vector</uint64_t>" ||
           tn == "/std/vector</int8_t>" || tn == "/std/vector</int16_t>" ||
           tn == "/std/vector</int32_t>" || tn == "/std/vector</int64_t>" ||
           tn == "/std/vector</float>" || tn == "/std/vector</double>" ||
           tn == "/base/Angle" || tn == "/base/Vector3d" ||
           tn == "/base/VectorXd" || tn == "/base/Time";
}

rockdisplay::vizkitplugin::Widget *Plot2dPlugin::createWidget()
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

    Plot2dWidget *ivvh = new Plot2dWidget(imView, found_frame_method,
            found_distanceimage_method);

    return ivvh;
}

std::string Plot2dPlugin::getName()
{
    return "Plot2d";
}

unsigned Plot2dPlugin::getFlags() {
    return Flags::WidgetCanHandleMultipleFields | Flags::KeepOpenWithoutFields |
    Flags::CanRemoveFields; //TODO CanRemoveFields also needs implementation of the removal functions to remove the graphs
}


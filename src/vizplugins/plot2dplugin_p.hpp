
#pragma once

#include "vizkitplugin.hpp"
#include <QMetaMethod>
#include <base/Time.hpp>
#include <unordered_map>
#include <QTimer>
#include <QVector>
#include <QColor>

QT_BEGIN_NAMESPACE

class QMouseEvent;

QT_END_NAMESPACE

namespace rock_display {

class Plot2dPlugin;
class Plot2dWidget;

class Plot2dOutputPortField : public rockdisplay::vizkitplugin::Field {
    Q_OBJECT;
private:
    Plot2dWidget *widget;
public:
    Plot2dOutputPortField(Plot2dWidget *widget, QObject *parent = nullptr);
public slots:
    virtual void updateOutputPort(const rockdisplay::vizkitplugin::ValueHandle *value) override;
};

class Plot2dInputPortField : public rockdisplay::vizkitplugin::Field {
    Q_OBJECT;
private:
    Plot2dWidget *widget;
public:
    Plot2dInputPortField(Plot2dWidget *widget, QObject *parent = nullptr);
public slots:
    virtual void updateInputPort(rockdisplay::vizkitplugin::ValueHandle *value) override;
};

class Plot2dPropertyField : public rockdisplay::vizkitplugin::Field {
    Q_OBJECT;
private:
    Plot2dWidget *widget;
public:
    Plot2dPropertyField(Plot2dWidget *widget, QObject *parent = nullptr);
public slots:
    virtual void updateProperty(rockdisplay::vizkitplugin::ValueHandle *value) override;
};

class Plot2dWidget : public rockdisplay::vizkitplugin::Widget {
    Q_OBJECT;
private:
    enum GraphStyle {
        Dot, Line
    };

    struct Options {
        bool auto_scrolling;
        bool auto_scrolling_y;
        bool auto_scrolling_x;
        float time_window;
        float cached_time_window;
        float pre_time_window;
        float xaxis_window;
        float pre_xaxis_window;
        float yaxis_window;
        float pre_yaxis_window;
        unsigned max_points;

        QVector<QColor> colors;
        bool reuse;
        bool use_y_axis2;
        GraphStyle plot_style;
        bool multi_use_menu;
        float update_period;
        // # this prevents repainting for each new sample
        bool plot_timestamps;
        bool is_time_plot;
        Options();
        ~Options();
    };

    struct GraphInfo {
        QObject* graph;
        bool use_y_axis2;
        QColor color;
        GraphStyle style;
        unsigned num_elements;//0,1,2 or more
    };

    QWidget *widget;
    QMetaMethod frame_method;
    QMetaMethod distanceimage_method;
    Plot2dOutputPortField *outputportfield;
    Plot2dInputPortField *inputportfield;
    Plot2dPropertyField *propertyfield;

    base::Time time0;
    bool time0_valid;
    base::Time current_time();
    QTimer *timer;
    std::unordered_map<std::string, base::Time> previous_time;
    std::unordered_map<std::string, GraphInfo> graphs;
    bool needs_update;
    Options options;
    unsigned color_next_idx;
    bool y1_needs_shrink;
    bool y2_needs_shrink;

    friend class Plot2dPlugin;


    void graph_style(QObject *graph, GraphStyle style);
    void plot_style(GraphStyle style);
    void rename_graph(std::string const &old_name, std::string const &new_name);
    QObject* graph2(std::string const &name);
    void rescale_yaxis(bool y1_axis = true, bool y2_axis = true);
    bool config(const rockdisplay::vizkitplugin::FieldDescription *type,
                std::string const &subpluginname);
    void update_zoom_range_flag(bool flag, bool use_2nd_axis);
    void update(double sample, std::string const &name, base::Time const &time);
    void update_vector(std::vector<double> const &samples, std::string const &name);
public:
    Plot2dWidget(QWidget *widget, QMetaMethod frame_method, QMetaMethod distanceimage_method, QObject *parent = nullptr);
    virtual QWidget *getWidget() override;
    void doUpdate(const rockdisplay::vizkitplugin::ValueHandle *data);
public slots:
    virtual rockdisplay::vizkitplugin::Field *addOutputPortField(const rockdisplay::vizkitplugin::FieldDescription *type, std::string const &subpluginname) override;
    virtual rockdisplay::vizkitplugin::Field *addInputPortField(const rockdisplay::vizkitplugin::FieldDescription *type, std::string const &subpluginname) override;
    virtual rockdisplay::vizkitplugin::Field *addPropertyField(const rockdisplay::vizkitplugin::FieldDescription *type, std::string const &subpluginname) override;

private slots:
    void mousePressOnPlotArea(QMouseEvent* event);
    void mousePressOnLegendItem(QMouseEvent* event, QVariant itemIdx);
};


}

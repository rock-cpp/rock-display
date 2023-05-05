
#pragma once

#include "vizkitplugin.hpp"
#include <QMetaMethod>

namespace rock_display {

class StreamAlignerWidgetPlugin;

class StreamAlignerWidgetOutputPortField : public rockdisplay::vizkitplugin::Field {
    Q_OBJECT;
private:
    QWidget *widget;
    QMetaMethod method;
public:
    StreamAlignerWidgetOutputPortField(QWidget *widget, QMetaMethod method, QObject *parent = nullptr);
public slots:
    virtual void updateOutputPort(const rockdisplay::vizkitplugin::ValueHandle *value) override;
};

class StreamAlignerWidgetInputPortField : public rockdisplay::vizkitplugin::Field {
    Q_OBJECT;
private:
    QWidget *widget;
    QMetaMethod method;
public:
    StreamAlignerWidgetInputPortField(QWidget *widget, QMetaMethod method, QObject *parent = nullptr);
public slots:
    virtual void updateInputPort(rockdisplay::vizkitplugin::ValueHandle *value) override;
};

class StreamAlignerWidgetPropertyField : public rockdisplay::vizkitplugin::Field {
    Q_OBJECT;
private:
    QWidget *widget;
    QMetaMethod method;
public:
    StreamAlignerWidgetPropertyField(QWidget *widget, QMetaMethod method, QObject *parent = nullptr);
public slots:
    virtual void updateProperty(rockdisplay::vizkitplugin::ValueHandle *value) override;
};

class StreamAlignerWidgetWidget : public rockdisplay::vizkitplugin::Widget {
    Q_OBJECT;
private:
    QWidget *widget;
    QMetaMethod method;
    StreamAlignerWidgetOutputPortField *outputportfield;
    StreamAlignerWidgetInputPortField *inputportfield;
    StreamAlignerWidgetPropertyField *propertyfield;

    friend class StreamAlignerWidgetPlugin;
public:
    StreamAlignerWidgetWidget(QWidget *widget, QMetaMethod method, QObject *parent = nullptr);
    virtual QWidget *getWidget() override;
public slots:
    virtual rockdisplay::vizkitplugin::Field *addOutputPortField(const rockdisplay::vizkitplugin::FieldDescription *type, std::string const &subpluginname) override;
    virtual rockdisplay::vizkitplugin::Field *addInputPortField(const rockdisplay::vizkitplugin::FieldDescription *type, std::string const &subpluginname) override;
    virtual rockdisplay::vizkitplugin::Field *addPropertyField(const rockdisplay::vizkitplugin::FieldDescription *type, std::string const &subpluginname) override;


};


}

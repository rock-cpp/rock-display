
#pragma once

#include "vizkitplugin.hpp"
#include <QMetaMethod>

namespace rock_display {

class ImageViewPlugin;

class ImageViewOutputPortField : public rockdisplay::vizkitplugin::Field {
    Q_OBJECT;
private:
    QWidget *widget;
    QMetaMethod method;
public:
    ImageViewOutputPortField(QWidget *widget, QMetaMethod method, QObject *parent = nullptr);
public slots:
    virtual void updateOutputPort(const rockdisplay::vizkitplugin::ValueHandle *value) override;
};

class ImageViewInputPortField : public rockdisplay::vizkitplugin::Field {
    Q_OBJECT;
private:
    QWidget *widget;
    QMetaMethod method;
public:
    ImageViewInputPortField(QWidget *widget, QMetaMethod method, QObject *parent = nullptr);
public slots:
    virtual void updateInputPort(rockdisplay::vizkitplugin::ValueHandle *value) override;
};

class ImageViewPropertyField : public rockdisplay::vizkitplugin::Field {
    Q_OBJECT;
private:
    QWidget *widget;
    QMetaMethod method;
public:
    ImageViewPropertyField(QWidget *widget, QMetaMethod method, QObject *parent = nullptr);
public slots:
    virtual void updateProperty(rockdisplay::vizkitplugin::ValueHandle *value) override;
};

class ImageViewWidget : public rockdisplay::vizkitplugin::Widget {
    Q_OBJECT;
private:
    QWidget *widget;
    QMetaMethod method;
    ImageViewOutputPortField *outputportfield;
    ImageViewInputPortField *inputportfield;
    ImageViewPropertyField *propertyfield;

    friend class ImageViewPlugin;
public:
    ImageViewWidget(QWidget *widget, QMetaMethod method, QObject *parent = nullptr);
    virtual QWidget *getWidget() override;
public slots:
    virtual rockdisplay::vizkitplugin::Field *addOutputPortField(const rockdisplay::vizkitplugin::FieldDescription *type, std::string const &subpluginname) override;
    virtual rockdisplay::vizkitplugin::Field *addInputPortField(const rockdisplay::vizkitplugin::FieldDescription *type, std::string const &subpluginname) override;
    virtual rockdisplay::vizkitplugin::Field *addPropertyField(const rockdisplay::vizkitplugin::FieldDescription *type, std::string const &subpluginname) override;
};


}

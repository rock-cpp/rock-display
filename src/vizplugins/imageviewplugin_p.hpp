
#pragma once

#include "vizkitplugin.hpp"
#include <QMetaMethod>

namespace rock_display {

class ImageViewPlugin;
class ImageViewWidget;

class ImageViewOutputPortField : public rockdisplay::vizkitplugin::Field {
    Q_OBJECT;
private:
    ImageViewWidget *widget;
public:
    ImageViewOutputPortField(ImageViewWidget *widget, QObject *parent = nullptr);
public slots:
    virtual void updateOutputPort(const rockdisplay::vizkitplugin::ValueHandle *value) override;
};

class ImageViewInputPortField : public rockdisplay::vizkitplugin::Field {
    Q_OBJECT;
private:
    ImageViewWidget *widget;
public:
    ImageViewInputPortField(ImageViewWidget *widget, QObject *parent = nullptr);
public slots:
    virtual void updateInputPort(rockdisplay::vizkitplugin::ValueHandle *value) override;
};

class ImageViewPropertyField : public rockdisplay::vizkitplugin::Field {
    Q_OBJECT;
private:
    ImageViewWidget *widget;
public:
    ImageViewPropertyField(ImageViewWidget *widget, QObject *parent = nullptr);
public slots:
    virtual void updateProperty(rockdisplay::vizkitplugin::ValueHandle *value) override;
};

class ImageViewWidget : public rockdisplay::vizkitplugin::Widget {
    Q_OBJECT;
private:
    QWidget *widget;
    QMetaMethod frame_method;
    QMetaMethod distanceimage_method;
    ImageViewOutputPortField *outputportfield;
    ImageViewInputPortField *inputportfield;
    ImageViewPropertyField *propertyfield;

    friend class ImageViewPlugin;
public:
    ImageViewWidget(QWidget *widget, QMetaMethod frame_method, QMetaMethod distanceimage_method, QObject *parent = nullptr);
    virtual QWidget *getWidget() override;
    void doUpdate(const rockdisplay::vizkitplugin::ValueHandle *data);
public slots:
    virtual rockdisplay::vizkitplugin::Field *addOutputPortField(const rockdisplay::vizkitplugin::FieldDescription *type, std::string const &subpluginname) override;
    virtual rockdisplay::vizkitplugin::Field *addInputPortField(const rockdisplay::vizkitplugin::FieldDescription *type, std::string const &subpluginname) override;
    virtual rockdisplay::vizkitplugin::Field *addPropertyField(const rockdisplay::vizkitplugin::FieldDescription *type, std::string const &subpluginname) override;
};


}

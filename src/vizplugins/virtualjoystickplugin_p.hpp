
#pragma once

#include "vizkitplugin.hpp"

namespace rock_display {

class VirtualJoystickInputPortField : public rockdisplay::vizkitplugin::Field {
    Q_OBJECT;
private:
    QWidget *widget;
    rockdisplay::vizkitplugin::ValueHandle *value;
public:
    VirtualJoystickInputPortField(QWidget *widget, QObject *parent = nullptr);
public slots:
    virtual void updateInputPort(rockdisplay::vizkitplugin::ValueHandle *value) override;
    void axisChanged(double x, double y);
};

class VirtualJoystickPropertyField : public rockdisplay::vizkitplugin::Field {
    Q_OBJECT;
private:
    QWidget *widget;
    rockdisplay::vizkitplugin::ValueHandle *value;
public:
    VirtualJoystickPropertyField(QWidget *widget, QObject *parent = nullptr);
public slots:
    virtual void updateProperty(rockdisplay::vizkitplugin::ValueHandle *value) override;
    void axisChanged(double x, double y);
};

class VirtualJoystickWidget : public rockdisplay::vizkitplugin::Widget {
    Q_OBJECT;
private:
    QWidget *widget;
    VirtualJoystickInputPortField *inputportfield;
    VirtualJoystickPropertyField *propertyfield;

    friend class NewVirtualJoystickPlugin;
public:
    VirtualJoystickWidget(QWidget *widget, QObject *parent = nullptr);
    virtual QWidget *getWidget() override;
public slots:
    virtual rockdisplay::vizkitplugin::Field *addOutputPortField(const rockdisplay::vizkitplugin::FieldDescription *type, std::string const &subpluginname) override;
    virtual rockdisplay::vizkitplugin::Field *addInputPortField(const rockdisplay::vizkitplugin::FieldDescription *type, std::string const &subpluginname) override;
    virtual rockdisplay::vizkitplugin::Field *addPropertyField(const rockdisplay::vizkitplugin::FieldDescription *type, std::string const &subpluginname) override;
    virtual void taskAvailable(
        rockdisplay::vizkitplugin::FieldDescription const *fieldDesc,
        rockdisplay::vizkitplugin::Field *field,
        bool available) override;
};

}

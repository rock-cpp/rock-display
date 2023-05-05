
#pragma once

#include "vizkitplugin.hpp"
#include <QMetaMethod>

namespace rock_display {

class SonarViewPlugin;
class SonarViewWidget;

class SonarViewOutputPortField : public rockdisplay::vizkitplugin::Field {
    Q_OBJECT;
private:
    SonarViewWidget *widget;
public:
    SonarViewOutputPortField(SonarViewWidget *widget, QObject *parent = nullptr);
public slots:
    virtual void updateOutputPort(const rockdisplay::vizkitplugin::ValueHandle *value) override;
};

class SonarViewInputPortField : public rockdisplay::vizkitplugin::Field {
    Q_OBJECT;
private:
    SonarViewWidget *widget;
public:
    SonarViewInputPortField(SonarViewWidget *widget, QObject *parent = nullptr);
public slots:
    virtual void updateInputPort(rockdisplay::vizkitplugin::ValueHandle *value) override;
};

class SonarViewPropertyField : public rockdisplay::vizkitplugin::Field {
    Q_OBJECT;
private:
    SonarViewWidget *widget;
public:
    SonarViewPropertyField(SonarViewWidget *widget, QObject *parent = nullptr);
public slots:
    virtual void updateProperty(rockdisplay::vizkitplugin::ValueHandle *value) override;
};

class SonarViewWidget : public rockdisplay::vizkitplugin::Widget {
    Q_OBJECT;
private:
    QWidget *widget;
    QMetaMethod addText_method;
    QMetaMethod setSonarScan_method;
    QMetaMethod update2_method;
    SonarViewOutputPortField *outputportfield;
    SonarViewInputPortField *inputportfield;
    SonarViewPropertyField *propertyfield;

    friend class SonarViewPlugin;

    bool init;
    QObject *time_overlay_object;

public:
    SonarViewWidget(QWidget *widget, QMetaMethod addText_method, QMetaMethod setSonarScan_method, QMetaMethod update2_method, QObject *parent = nullptr);
    virtual QWidget *getWidget() override;
    void doUpdate(rockdisplay::vizkitplugin::ValueHandle const *data);
public slots:
    virtual rockdisplay::vizkitplugin::Field *addOutputPortField(const rockdisplay::vizkitplugin::FieldDescription *type, std::string const &subpluginname) override;
    virtual rockdisplay::vizkitplugin::Field *addInputPortField(const rockdisplay::vizkitplugin::FieldDescription *type, std::string const &subpluginname) override;
    virtual rockdisplay::vizkitplugin::Field *addPropertyField(const rockdisplay::vizkitplugin::FieldDescription *type, std::string const &subpluginname) override;
};


}

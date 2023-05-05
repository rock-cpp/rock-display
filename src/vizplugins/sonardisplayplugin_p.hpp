
#pragma once

#include "vizkitplugin.hpp"
#include <QMetaMethod>

namespace rock_display {

class SonarDisplayPlugin;
class SonarDisplayWidget;

class SonarDisplayOutputPortField : public rockdisplay::vizkitplugin::Field {
    Q_OBJECT;
private:
    SonarDisplayWidget *widget;
public:
    SonarDisplayOutputPortField(SonarDisplayWidget *widget, QObject *parent = nullptr);
public slots:
    virtual void updateOutputPort(const rockdisplay::vizkitplugin::ValueHandle *value) override;
};

class SonarDisplayInputPortField : public rockdisplay::vizkitplugin::Field {
    Q_OBJECT;
private:
    SonarDisplayWidget *widget;
public:
    SonarDisplayInputPortField(SonarDisplayWidget *widget, QObject *parent = nullptr);
public slots:
    virtual void updateInputPort(rockdisplay::vizkitplugin::ValueHandle *value) override;
};

class SonarDisplayPropertyField : public rockdisplay::vizkitplugin::Field {
    Q_OBJECT;
private:
    SonarDisplayWidget *widget;
public:
    SonarDisplayPropertyField(SonarDisplayWidget *widget, QObject *parent = nullptr);
public slots:
    virtual void updateProperty(rockdisplay::vizkitplugin::ValueHandle *value) override;
};

class SonarDisplayWidget : public rockdisplay::vizkitplugin::Widget {
    Q_OBJECT;
private:
    QWidget *widget;
    QMetaMethod addSonarBeam_method;
    QMetaMethod setUpSonar_method;
    SonarDisplayOutputPortField *outputportfield;
    SonarDisplayInputPortField *inputportfield;
    SonarDisplayPropertyField *propertyfield;
    unsigned number_of_bins;
    double resolution;

    friend class SonarDisplayPlugin;
public:
    SonarDisplayWidget(QWidget *widget, QMetaMethod addSonarBeam_method, QMetaMethod setUpSonar_method, QObject *parent = nullptr);
    virtual QWidget *getWidget() override;

    void updateData(rockdisplay::vizkitplugin::ValueHandle const *data);
public slots:
    virtual rockdisplay::vizkitplugin::Field *addOutputPortField(const rockdisplay::vizkitplugin::FieldDescription *type, std::string const &subpluginname) override;
    virtual rockdisplay::vizkitplugin::Field *addInputPortField(const rockdisplay::vizkitplugin::FieldDescription *type, std::string const &subpluginname) override;
    virtual rockdisplay::vizkitplugin::Field *addPropertyField(const rockdisplay::vizkitplugin::FieldDescription *type, std::string const &subpluginname) override;
};


}

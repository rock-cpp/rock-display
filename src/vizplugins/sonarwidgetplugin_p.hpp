
#pragma once

#include "vizkitplugin.hpp"
#include <QMetaMethod>

namespace rock_display {

class SonarWidgetPlugin;

class SonarWidgetOutputPortField : public rockdisplay::vizkitplugin::Field {
    Q_OBJECT;
private:
    QWidget *widget;
    QMetaMethod Sonar_method;
    QMetaMethod SonarScan_method;
public:
    SonarWidgetOutputPortField(QWidget *widget,
                                     QMetaMethod const &Sonar_method,
                                     QMetaMethod const &SonarScan_method,
                                     QObject *parent = nullptr);
public slots:
    virtual void updateOutputPort(const rockdisplay::vizkitplugin::ValueHandle *value) override;
};

class SonarWidgetInputPortField : public rockdisplay::vizkitplugin::Field {
    Q_OBJECT;
private:
    QWidget *widget;
    QMetaMethod Sonar_method;
    QMetaMethod SonarScan_method;
public:
    SonarWidgetInputPortField(QWidget *widget,
                                    QMetaMethod const &Sonar_method,
                                    QMetaMethod const &SonarScan_method,
                                    QObject *parent = nullptr);
public slots:
    virtual void updateInputPort(rockdisplay::vizkitplugin::ValueHandle *value) override;
};

class SonarWidgetPropertyField : public rockdisplay::vizkitplugin::Field {
    Q_OBJECT;
private:
    QWidget *widget;
    QMetaMethod Sonar_method;
    QMetaMethod SonarScan_method;
public:
    SonarWidgetPropertyField(QWidget *widget,
                                   QMetaMethod const &Sonar_method,
                                   QMetaMethod const &SonarScan_method,
                                   QObject *parent = nullptr);
public slots:
    virtual void updateProperty(rockdisplay::vizkitplugin::ValueHandle *value) override;
};

class SonarWidgetWidget : public rockdisplay::vizkitplugin::Widget {
    Q_OBJECT;
private:
    QWidget *widget;
    QMetaMethod Sonar_method;
    QMetaMethod SonarScan_method;
    SonarWidgetOutputPortField *outputportfield;
    SonarWidgetInputPortField *inputportfield;
    SonarWidgetPropertyField *propertyfield;

    friend class SonarWidgetPlugin;
public:
    SonarWidgetWidget(QWidget *widget,
                            QMetaMethod const &Sonar_method,
                            QMetaMethod const &SonarScan_method,
                            QObject *parent = nullptr);
    virtual QWidget *getWidget() override;
public slots:
    virtual rockdisplay::vizkitplugin::Field *addOutputPortField(const rockdisplay::vizkitplugin::FieldDescription *type, std::string const &subpluginname) override;
    virtual rockdisplay::vizkitplugin::Field *addInputPortField(const rockdisplay::vizkitplugin::FieldDescription *type, std::string const &subpluginname) override;
    virtual rockdisplay::vizkitplugin::Field *addPropertyField(const rockdisplay::vizkitplugin::FieldDescription *type, std::string const &subpluginname) override;
};


}


#pragma once

#include "vizkitplugin.hpp"
#include <QMetaMethod>

namespace rock_display {

class OrientationViewPlugin;

class OrientationViewOutputPortField : public rockdisplay::vizkitplugin::Field {
    Q_OBJECT;
private:
    QWidget *widget;
    QMetaMethod setPitchAngle_method;
    QMetaMethod setRollAngle_method;
    QMetaMethod setHeadingAngle_method;
public:
    OrientationViewOutputPortField(QWidget *widget,
                                   QMetaMethod const &setPitchAngle_method,
                                   QMetaMethod const &setRollAngle_method,
                                   QMetaMethod const &setHeadingAngle_method,
                                   QObject *parent = nullptr);
public slots:
    virtual void updateOutputPort(const rockdisplay::vizkitplugin::ValueHandle *value) override;
};

class OrientationViewInputPortField : public rockdisplay::vizkitplugin::Field {
    Q_OBJECT;
private:
    QWidget *widget;
    QMetaMethod setPitchAngle_method;
    QMetaMethod setRollAngle_method;
    QMetaMethod setHeadingAngle_method;
public:
    OrientationViewInputPortField(QWidget *widget,
                                  QMetaMethod const &setPitchAngle_method,
                                  QMetaMethod const &setRollAngle_method,
                                  QMetaMethod const &setHeadingAngle_method,
                                  QObject *parent = nullptr);
public slots:
    virtual void updateInputPort(rockdisplay::vizkitplugin::ValueHandle *value) override;
};

class OrientationViewPropertyField : public rockdisplay::vizkitplugin::Field {
    Q_OBJECT;
private:
    QWidget *widget;
    QMetaMethod setPitchAngle_method;
    QMetaMethod setRollAngle_method;
    QMetaMethod setHeadingAngle_method;
public:
    OrientationViewPropertyField(QWidget *widget,
                                 QMetaMethod const &setPitchAngle_method,
                                 QMetaMethod const &setRollAngle_method,
                                 QMetaMethod const &setHeadingAngle_method,
                                 QObject *parent = nullptr);
public slots:
    virtual void updateProperty(rockdisplay::vizkitplugin::ValueHandle *value) override;
};

class OrientationViewWidget : public rockdisplay::vizkitplugin::Widget {
    Q_OBJECT;
private:
    QWidget *widget;
    QMetaMethod setPitchAngle_method;
    QMetaMethod setRollAngle_method;
    QMetaMethod setHeadingAngle_method;
    OrientationViewOutputPortField *outputportfield;
    OrientationViewInputPortField *inputportfield;
    OrientationViewPropertyField *propertyfield;

    friend class OrientationViewPlugin;
public:
    OrientationViewWidget(QWidget *widget,
                          QMetaMethod const &setPitchAngle_method,
                          QMetaMethod const &setRollAngle_method,
                          QMetaMethod const &setHeadingAngle_method,
                          QObject *parent = nullptr);
    virtual QWidget *getWidget() override;
public slots:
    virtual rockdisplay::vizkitplugin::Field *addOutputPortField(const rockdisplay::vizkitplugin::FieldDescription *type, std::string const &subpluginname) override;
    virtual rockdisplay::vizkitplugin::Field *addInputPortField(const rockdisplay::vizkitplugin::FieldDescription *type, std::string const &subpluginname) override;
    virtual rockdisplay::vizkitplugin::Field *addPropertyField(const rockdisplay::vizkitplugin::FieldDescription *type, std::string const &subpluginname) override;
};


}


#pragma once

#include "vizkitplugin.hpp"
#include <QMetaMethod>

namespace rock_display {

class ArtificialHorizonPlugin;

class ArtificialHorizonOutputPortField : public rockdisplay::vizkitplugin::Field {
    Q_OBJECT;
private:
    QWidget *widget;
    QMetaMethod setPitchAngle_method;
    QMetaMethod setRollAngle_method;
public:
    ArtificialHorizonOutputPortField(QWidget *widget,
                                     QMetaMethod const &setPitchAngle_method,
                                     QMetaMethod const &setRollAngle_method,
                                     QObject *parent = nullptr);
public slots:
    virtual void updateOutputPort(const rockdisplay::vizkitplugin::ValueHandle *value) override;
};

class ArtificialHorizonInputPortField : public rockdisplay::vizkitplugin::Field {
    Q_OBJECT;
private:
    QWidget *widget;
    QMetaMethod setPitchAngle_method;
    QMetaMethod setRollAngle_method;
public:
    ArtificialHorizonInputPortField(QWidget *widget,
                                    QMetaMethod const &setPitchAngle_method,
                                    QMetaMethod const &setRollAngle_method,
                                    QObject *parent = nullptr);
public slots:
    virtual void updateInputPort(rockdisplay::vizkitplugin::ValueHandle *value) override;
};

class ArtificialHorizonPropertyField : public rockdisplay::vizkitplugin::Field {
    Q_OBJECT;
private:
    QWidget *widget;
    QMetaMethod setPitchAngle_method;
    QMetaMethod setRollAngle_method;
public:
    ArtificialHorizonPropertyField(QWidget *widget,
                                   QMetaMethod const &setPitchAngle_method,
                                   QMetaMethod const &setRollAngle_method,
                                   QObject *parent = nullptr);
public slots:
    virtual void updateProperty(rockdisplay::vizkitplugin::ValueHandle *value) override;
};

class ArtificialHorizonWidget : public rockdisplay::vizkitplugin::Widget {
    Q_OBJECT;
private:
    QWidget *widget;
    QMetaMethod setPitchAngle_method;
    QMetaMethod setRollAngle_method;
    ArtificialHorizonOutputPortField *outputportfield;
    ArtificialHorizonInputPortField *inputportfield;
    ArtificialHorizonPropertyField *propertyfield;

    friend class ArtificialHorizonPlugin;
public:
    ArtificialHorizonWidget(QWidget *widget,
                            QMetaMethod const &setPitchAngle_method,
                            QMetaMethod const &setRollAngle_method,
                            QObject *parent = nullptr);
    virtual QWidget *getWidget() override;
public slots:
    virtual rockdisplay::vizkitplugin::Field *addOutputPortField(const rockdisplay::vizkitplugin::FieldDescription *type, std::string const &subpluginname) override;
    virtual rockdisplay::vizkitplugin::Field *addInputPortField(const rockdisplay::vizkitplugin::FieldDescription *type, std::string const &subpluginname) override;
    virtual rockdisplay::vizkitplugin::Field *addPropertyField(const rockdisplay::vizkitplugin::FieldDescription *type, std::string const &subpluginname) override;
};


}

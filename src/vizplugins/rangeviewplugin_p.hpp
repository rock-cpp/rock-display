
#pragma once

#include "vizkitplugin.hpp"
#include <QMetaMethod>

namespace rock_display {

class RangeViewPlugin;

class RangeViewOutputPortField : public rockdisplay::vizkitplugin::Field {
    Q_OBJECT;
private:
    QWidget *widget;
    QMetaMethod method;
public:
    RangeViewOutputPortField(QWidget *widget,
                                     QMetaMethod const &method,
                                     QObject *parent = nullptr);
public slots:
    virtual void updateOutputPort(const rockdisplay::vizkitplugin::ValueHandle *value) override;
};

class RangeViewInputPortField : public rockdisplay::vizkitplugin::Field {
    Q_OBJECT;
private:
    QWidget *widget;
    QMetaMethod method;
public:
    RangeViewInputPortField(QWidget *widget,
                                    QMetaMethod const &method,
                                    QObject *parent = nullptr);
public slots:
    virtual void updateInputPort(rockdisplay::vizkitplugin::ValueHandle *value) override;
};

class RangeViewPropertyField : public rockdisplay::vizkitplugin::Field {
    Q_OBJECT;
private:
    QWidget *widget;
    QMetaMethod method;
public:
    RangeViewPropertyField(QWidget *widget,
                                   QMetaMethod const &method,
                                   QObject *parent = nullptr);
public slots:
    virtual void updateProperty(rockdisplay::vizkitplugin::ValueHandle *value) override;
};

class RangeViewWidget : public rockdisplay::vizkitplugin::Widget {
    Q_OBJECT;
private:
    QWidget *widget;
    QMetaMethod method;
    RangeViewOutputPortField *outputportfield;
    RangeViewInputPortField *inputportfield;
    RangeViewPropertyField *propertyfield;

    friend class RangeViewPlugin;
public:
    RangeViewWidget(QWidget *widget,
                            QMetaMethod const &method,
                            QObject *parent = nullptr);
    virtual QWidget *getWidget() override;
public slots:
    virtual rockdisplay::vizkitplugin::Field *addOutputPortField(const rockdisplay::vizkitplugin::FieldDescription *type, std::string const &subpluginname) override;
    virtual rockdisplay::vizkitplugin::Field *addInputPortField(const rockdisplay::vizkitplugin::FieldDescription *type, std::string const &subpluginname) override;
    virtual rockdisplay::vizkitplugin::Field *addPropertyField(const rockdisplay::vizkitplugin::FieldDescription *type, std::string const &subpluginname) override;
};


}

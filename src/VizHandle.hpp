
#pragma once

#include <QObject>

QT_BEGIN_NAMESPACE
class QWidget;
QT_END_NAMESPACE

class VizHandle : public QObject
{
    Q_OBJECT
public:
    virtual ~VizHandle() {}
    virtual QObject *getVizkit3dPluginObject() = 0;
    virtual QWidget *getStandaloneWidget() = 0;
public slots:
    virtual void updateVisualizer(void const *data){}
    /* this sample can be kept around for editing purposes */
    virtual void updateEditable(void *data){}
signals:
    void editableChanged(void *data, bool force_send = false);
    void closing(VizHandle *vh);
};


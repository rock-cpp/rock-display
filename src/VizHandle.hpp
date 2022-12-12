
#pragma once

#include <QObject>
#include <rtt/base/DataSourceBase.hpp>

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
    virtual void updateVisualizer(void const *data, RTT::base::DataSourceBase::shared_ptr base_sample){}
    /* this sample can be kept around for editing purposes */
    virtual void updateEditable(void *data, RTT::base::DataSourceBase::shared_ptr base_sample){}
signals:
    void editableChanged(void *data, RTT::base::DataSourceBase::shared_ptr base_sample,bool force_send = false);
    void closing(VizHandle *vh);
};



#pragma once

#include "../Vizkit3dPluginRepository.hpp"

class VirtualJoystickVizHandle : public VizHandle
{
    Q_OBJECT;
public:
    QWidget *widget;
    void *data;
    RTT::base::DataSourceBase::shared_ptr base_sample;
    virtual QObject *getVizkit3dPluginObject() override { return nullptr; }
    virtual QWidget *getStandaloneWidget() override { return widget; }
public slots:
    virtual void updateEditable(void *data) override;
    virtual void axisChanged(double x, double y);
protected:
    virtual bool eventFilter(QObject *obj, QEvent *event) override;
};


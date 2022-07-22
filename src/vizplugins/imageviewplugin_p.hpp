
#pragma once

#include "../Vizkit3dPluginRepository.hpp"

class ImageViewVizHandle : public VizHandle
{
    Q_OBJECT;
public:
    QWidget *widget;
    QMetaMethod method;
    virtual QObject *getVizkit3dPluginObject() override { return nullptr; }
    virtual QWidget *getStandaloneWidget() override { return widget; }
public slots:
    virtual void updateVisualizer(void const *data, RTT::base::DataSourceBase::shared_ptr base_sample) override;
};


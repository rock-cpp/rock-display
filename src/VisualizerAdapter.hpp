#pragma once

#include "Vizkit3dPluginRepository.hpp"
#include <qobject.h>
#include <map>
#include <rtt/base/DataSourceBase.hpp>

class VizHandle;

class VisualizerAdapter : public QObject
{
    Q_OBJECT
    
protected:
    std::map<std::string, VizHandle> visualizers;
    
public:
    VisualizerAdapter()
    {
    }
    
    virtual ~VisualizerAdapter()
    {
    }
    
    virtual void addPlugin(const std::string &name, VizHandle handle);
    bool hasVisualizer(const std::string &name);
    QObject *getVisualizer(const std::string &name);
    bool removeVisualizer(QObject *plugin);
    virtual bool hasVisualizers()
    {
        return visualizers.empty();
    }
    void updateVisualizer(VizHandle vizhandle, RTT::base::DataSourceBase::shared_ptr data)
    {
        emit requestVisualizerUpdate(vizhandle, data);
    }
    
signals:
    void requestVisualizerUpdate(VizHandle vizhandle, RTT::base::DataSourceBase::shared_ptr data);
};
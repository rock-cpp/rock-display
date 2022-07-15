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
    std::map<std::string, VizHandle*> visualizers;
    
public:
    VisualizerAdapter()
    {
    }
    
    virtual ~VisualizerAdapter()
    {
    }
    
    virtual void addPlugin(const std::string &name, VizHandle *handle);
    bool hasVisualizer(const std::string &name);
    VizHandle *getVisualizer(const std::string &name);
    void removeVisualizer(VizHandle *plugin);
    virtual bool hasVisualizers()
    {
        return receivers(SIGNAL(visualizerUpdate(void const *, RTT::base::DataSourceBase::shared_ptr))) != 0;
    }
    
signals:
    /* alternatively, one could pass around "Typelib::Value" instead of "void const *",
     * retaining runtime type information */
    void visualizerUpdate(void const * data, RTT::base::DataSourceBase::shared_ptr base_sample);
};

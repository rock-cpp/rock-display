#pragma once

#include "Vizkit3dPluginRepository.hpp"
#include <qobject.h>
#include <map>

class VizHandle;

class VisualizerAdapter : public QObject
{
    Q_OBJECT
    
protected:
    std::map<std::string, VizHandle*> visualizers;
    std::mutex visualizerMutex;
    
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
        std::lock_guard<std::mutex> g(visualizerMutex);
        return receivers(SIGNAL(visualizerUpdate(void const *))) != 0;
    }
signals:
    /* *Update are used to pass data from the items/ports/properties to plugins
     *
     * alternatively, one could pass around "Typelib::Value" instead of "void const *",
     * retaining runtime type information */
    void visualizerUpdate(void const * data);
    void editableUpdate(void * data);
};

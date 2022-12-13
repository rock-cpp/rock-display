#pragma once

#include <qobject.h>
#include <map>

class VizHandle;

namespace Typelib
{
    class Value;
}

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
        return receivers(SIGNAL(visualizerUpdate(const Typelib::Value&))) != 0;
    }
signals:
    /* *Update are used to pass data from the items/ports/properties to plugins
     *
     */
    void visualizerUpdate(Typelib::Value const &value);
    void editableUpdate(Typelib::Value const &value);
};

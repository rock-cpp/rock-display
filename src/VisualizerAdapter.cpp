#include "VisualizerAdapter.hpp"
#include "VizHandle.hpp"

void VisualizerAdapter::addPlugin(const std::string &name, VizHandle *handle)
{
    visualizers.insert(std::make_pair(name, handle));
    connect(this, &VisualizerAdapter::visualizerUpdate,
            handle, &VizHandle::updateVisualizer);
    connect(this, &VisualizerAdapter::editableUpdate,
            handle, &VizHandle::updateEditable);
    //if this assert fails, the signature here and in hasVisualizer must be updated.
    assert(receivers(SIGNAL(visualizerUpdate(const Typelib::Value&))) != 0);
}

VizHandle *VisualizerAdapter::getVisualizer(const std::string& name)
{
    std::map<std::string, VizHandle*>::iterator iter = visualizers.find(name);
    if (iter != visualizers.end())
    {
        return iter->second;
    }
    
    return nullptr;
}

bool VisualizerAdapter::hasVisualizer(const std::string& name)
{
    return visualizers.find(name) != visualizers.end();
}

void VisualizerAdapter::removeVisualizer(VizHandle *plugin)
{
    for (std::map<std::string, VizHandle*>::iterator it = visualizers.begin(); it != visualizers.end(); it++)
    {
        if (it->second == plugin)
        {
            visualizers.erase(it);
            return;
        }
    }
}


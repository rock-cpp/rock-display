#include "VisualizerAdapter.hpp"

void VisualizerAdapter::addPlugin(const std::string &name, VizHandle handle)
{
    visualizers.insert(std::make_pair(name, handle));
}

QObject* VisualizerAdapter::getVisualizer(const std::string& name)
{
    std::map<std::string, VizHandle>::iterator iter = visualizers.find(name);
    if (iter != visualizers.end())
    {
        return iter->second.plugin;
    }
    
    return nullptr;
}

bool VisualizerAdapter::hasVisualizer(const std::string& name)
{
    return visualizers.find(name) != visualizers.end();
}

bool VisualizerAdapter::removeVisualizer(QObject* plugin)
{
    for (std::map<std::string, VizHandle>::iterator it = visualizers.begin(); it != visualizers.end(); it++)
    {
        if (it->second.plugin == plugin)
        {
            visualizers.erase(it);
            return true;
        }
    }
    
    return false;
}
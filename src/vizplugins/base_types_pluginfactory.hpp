
#pragma once

#include "vizkitplugin.hpp"

namespace rockdisplay {
namespace base_types
{
class PluginFactory : public rockdisplay::vizkitplugin::PluginFactory
{
    Q_OBJECT;
    Q_PLUGIN_METADATA(IID RockdisplayVizkitPluginFactory_iid)
    Q_INTERFACES(rockdisplay::vizkitplugin::PluginFactory)
public:
    virtual std::vector<rockdisplay::vizkitplugin::Plugin *> createPlugins();
};
}
}

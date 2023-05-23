
#pragma once

#include <QString>
#include <vector>

namespace rockdisplay
{
namespace vizkitplugin
{
class Plugin;
}

class PluginManager
{
public:
    PluginManager();
    std::vector< rockdisplay::vizkitplugin::Plugin* > vizkitplugins() {
        return m_vizkitplugins;
    }
private:
    void tryRegisterPlugin(QString const &plugin);
    std::vector< rockdisplay::vizkitplugin::Plugin* > m_vizkitplugins;
};
}

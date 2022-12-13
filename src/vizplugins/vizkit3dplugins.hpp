
#pragma once

#include "vizkitplugin.hpp"

class Vizkit3dPluginRepository;

namespace vizkit3d
{
class Vizkit3DWidget;
}

namespace rock_display {

class Vizkit3DPluginsWidget;

class Vizkit3DPlugins : public rockdisplay::vizkitplugin::Plugin {
    Q_OBJECT;
private:
    Vizkit3dPluginRepository *pluginRepo;
    vizkit3d::Vizkit3DWidget *v3dwidget;
    Vizkit3DPluginsWidget *currentWidget;
public:
    Vizkit3DPlugins();
    virtual ~Vizkit3DPlugins() override;
    virtual bool probeOutputPort(rockdisplay::vizkitplugin::FieldDescription *fieldDesc, std::vector<std::string> &names) override;
    virtual bool probeInputPort(rockdisplay::vizkitplugin::FieldDescription *fieldDesc, std::vector<std::string> &names) override;
    virtual bool probeProperty(rockdisplay::vizkitplugin::FieldDescription *fieldDesc, std::vector<std::string> &names) override;
    virtual rockdisplay::vizkitplugin::Widget *createWidget() override;
    virtual std::string getName() override;
    virtual unsigned getFlags() override;

    virtual std::vector<std::string> getStandaloneSubplugins() override;
};

}

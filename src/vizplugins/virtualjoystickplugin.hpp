
#pragma once

#include "vizkitplugin.hpp"

namespace rock_display {

class VirtualJoystickPlugin : public rockdisplay::vizkitplugin::Plugin {
    Q_OBJECT;
public:
    virtual bool probeOutputPort(rockdisplay::vizkitplugin::FieldDescription *fieldDesc, std::vector<std::string> &names) override;
    virtual bool probeInputPort(rockdisplay::vizkitplugin::FieldDescription *fieldDesc, std::vector<std::string> &names) override;
    virtual bool probeProperty(rockdisplay::vizkitplugin::FieldDescription *fieldDesc, std::vector<std::string> &names) override;
    virtual rockdisplay::vizkitplugin::Widget *createWidget() override;
    virtual std::string getName() override;
    virtual unsigned getFlags() override;
};

}

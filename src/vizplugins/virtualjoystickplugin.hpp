
#pragma once

#include "../Vizkit3dPluginRepository.hpp"

class VirtualJoystickPluginHandle : public PluginHandle
{
public:
    VirtualJoystickPluginHandle();
    std::string typeName;
    virtual VizHandle *createViz() const override;
    virtual bool probe(Typelib::Type const &type, const Typelib::Registry* registry = NULL) const override;
};


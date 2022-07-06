
#pragma once

#include "../Vizkit3dPluginRepository.hpp"

class ImageViewPluginHandle : public PluginHandle
{
public:
    ImageViewPluginHandle();
    std::string typeName;
    virtual VizHandle *createViz() const override;
};


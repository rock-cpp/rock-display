
#pragma once

#include "VizHandle.hpp"
#include <string>
#include <QObject>

namespace Typelib
{
class Registry;
class Type;
}

class PluginHandle : public QObject
{
public:
    std::string pluginName;
    PluginHandle(std::string const &pluginName) : pluginName(pluginName) {}
    virtual ~PluginHandle() {}
    virtual VizHandle *createViz() const = 0;
    virtual bool probe(Typelib::Type const &type, const Typelib::Registry* registry = NULL) const = 0;
};


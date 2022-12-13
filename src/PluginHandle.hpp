
#pragma once

#include "VizHandle.hpp"
#include <string>
#include <QObject>

namespace Typelib
{
class Registry;
class Type;
}

namespace RTT
{
namespace base
{
class OutputPortInterface;
class InputPortInterface;
class PropertyBase;
}
}

class TypedItem;

class PluginHandle : public QObject
{
public:
    std::string pluginName;
    PluginHandle(std::string const &pluginName) : pluginName(pluginName) {}
    virtual ~PluginHandle() {}
    virtual VizHandle *createViz() const = 0;
    virtual VizHandle *createViz(const std::string &taskName,
                                 RTT::base::OutputPortInterface *outputport,
                                 Typelib::Type const *type,
                                 const Typelib::Registry* registry,
                                 std::string const &fieldName,
                                 TypedItem* ti) const = 0;
    virtual VizHandle *createViz(const std::string &taskName,
                                 RTT::base::InputPortInterface *inputport,
                                 Typelib::Type const *type,
                                 const Typelib::Registry* registry,
                                 std::string const &fieldName,
                                 TypedItem* ti) const = 0;
    virtual VizHandle *createViz(const std::string &taskName,
                                 RTT::base::PropertyBase *property,
                                 Typelib::Type const *type,
                                 const Typelib::Registry* registry,
                                 std::string const &fieldName,
                                 TypedItem* ti) const = 0;
    virtual bool probeProperty(RTT::base::PropertyBase *property,
                               Typelib::Type const *type,
                               const Typelib::Registry *registry) const = 0;
    virtual bool probeInputPort(RTT::base::InputPortInterface *inputport,
                                Typelib::Type const *type,
                                const Typelib::Registry *registry) const = 0;
    virtual bool probeOutputPort(RTT::base::OutputPortInterface *outputport,
                                 Typelib::Type const *type,
                                 const Typelib::Registry *registry) const = 0;
};



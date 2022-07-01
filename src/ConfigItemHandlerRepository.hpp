
#pragma once

#include <vector>

namespace Typelib
{
    class Type;
}

class ConfigItemHandler;

class ConfigItemHandlerRepository
{
private:
    std::vector<ConfigItemHandler const*> configitemhandlers;

public:
    ConfigItemHandlerRepository();

    std::vector<ConfigItemHandler const*> findNameServiceConfigItemHandlersFor(Typelib::Type const &type, bool editing = true);

    void registerNameServiceConfigItemHandler( ConfigItemHandler const *handler);
};


#include "ConfigItemHandlerRepository.hpp"
#include "ConfigItemHandler.hpp"

#include "configitemhandlers/EnumHandler.hpp"

ConfigItemHandlerRepository::ConfigItemHandlerRepository()
{
    configitemhandlers.push_back(new EnumHandler());
}

std::vector<ConfigItemHandler const*> ConfigItemHandlerRepository::findNameServiceConfigItemHandlersFor(Typelib::Type const &type, bool editing)
{
    //maybe order them by some priority.
    std::vector<ConfigItemHandler const*> resultset;
    for(auto &h : configitemhandlers)
    {
        if (h->probe(type, editing))
            resultset.push_back(h);
    }

    return resultset;
}

void ConfigItemHandlerRepository::registerNameServiceConfigItemHandler( ConfigItemHandler const *handler)
{
    configitemhandlers.push_back(handler);
}


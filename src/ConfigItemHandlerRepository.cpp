
#include "ConfigItemHandlerRepository.hpp"
#include "ConfigItemHandler.hpp"

#include "configitemhandlers/EnumHandler.hpp"
#include "configitemhandlers/StdStringHandler.hpp"
#include "configitemhandlers/ContainerHandler.hpp"
#include "configitemhandlers/ImageCtxMenuHandler.hpp"
#include "configitemhandlers/BinaryCtxMenuHandler.hpp"

ConfigItemHandlerRepository::ConfigItemHandlerRepository()
{
    configitemhandlers.push_back(new EnumHandler());
    configitemhandlers.push_back(new StdStringHandler());
    configitemhandlers.push_back(new ContainerHandler());
    configitemhandlers.push_back(new ImageCtxMenuHandler());
    configitemhandlers.push_back(new BinaryCtxMenuHandler());
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


#include "Vizkit3dPluginRepository.hpp"
#include <base-logging/Logging.hpp>
#include <iostream>
#include <boost/regex.hpp>
#include <typelib/registry.hh>
#include <typelib/value.hh>
#include <rtt/base/OutputPortInterface.hpp>
#include <rtt/types/TypeInfo.hpp>

static std::string convertTypelibToDottedTypeName(std::string const &typelibTypeName)
{
    std::string dottedTypeName = typelibTypeName.substr(1, typelibTypeName.size());
    //the _m names and wrapper:: namespace is not guaranteed to be binary compatible.
    /*static boost::regex re1("_m");
    dottedTypeName = boost::regex_replace(dottedTypeName, re1, "");*/
    static boost::regex re2("</");
    dottedTypeName = boost::regex_replace(dottedTypeName, re2, "<");
    static boost::regex re3("/");
    dottedTypeName = boost::regex_replace(dottedTypeName, re3, "::");
    static boost::regex re4("\\s\\[.*\\]");
    dottedTypeName = boost::regex_replace(dottedTypeName, re4, "");
    static boost::regex re5(">>");
    dottedTypeName = boost::regex_replace(dottedTypeName, re5, "> >");
    return dottedTypeName;
}

static std::string convertDottedToTypelibTypeName(std::string const &dottedTypeName)
{
    std::string typelibTypeName = dottedTypeName;
    static boost::regex re1("> >");
    typelibTypeName = boost::regex_replace(typelibTypeName, re1, ">>");
    static boost::regex re2("::");
    typelibTypeName = boost::regex_replace(typelibTypeName, re2, "/");
    static boost::regex re3("<");
    typelibTypeName = boost::regex_replace(typelibTypeName, re3, "</");
    typelibTypeName = std::string("/")+typelibTypeName;
    return typelibTypeName;
}

Vizkit3dPluginHandle::Vizkit3dPluginHandle(std::string const &pluginName)
: pluginName(pluginName)
{
}

Vizkit3dPluginRepository::Vizkit3dPluginRepository(QStringList &plugins)
{
    QPluginLoader loader;
    std::set<std::string> libSet;
    for(QString &pluginString : plugins)
    {
        libSet.insert(pluginString.split('@').back().toStdString());
    }    
    
    for(const std::string &libPath: libSet)
    {
        loader.setFileName(libPath.c_str());
        if(!loader.load())
        {
            LOG_WARN_S << "Vizkit3dPluginRepository: failed to load libPath " << libPath << "..";
        }
        vizkit3d::VizkitPluginFactory *factory = dynamic_cast<vizkit3d::VizkitPluginFactory *>(loader.instance());
        if(!factory)
        {
            LOG_ERROR_S << "Vizkit3dPluginRepository: Internal error, plugin is not a factory..";
            continue;
        }
        
        QStringList *availablePlugins = factory->getAvailablePlugins();
                
        for(const QString &pName: *availablePlugins)
        {   
            std::map<std::string, Vizkit3dPluginHandle*> typeMap;
            QObject *plugin = factory->createPlugin(pName);
            LOG_INFO_S << "plugin " << pName.toStdString();
            
            if(!plugin)
            {
                std::cout << "warning: couldn't load plugin " << pName.toStdString() << std::endl;
                continue;
            }
            
            const QMetaObject *metaPlugin = plugin->metaObject();
        
            bool foundUpdateFunctions = false;
            
            for(int i = 0 ; i < metaPlugin->methodCount(); i++)
            {
                QMetaMethod method = metaPlugin->method(i);
                auto parameterList = method.parameterTypes();
                if(parameterList.size() != 1)
                    continue;
                
                std::string signature = method.methodSignature().toStdString();
                static const std::string update("update");
                
                if(signature.size() > update.size() && signature.substr(0, update.size()) == update)
                {
                    foundUpdateFunctions = true;
                    Vizkit3dPluginHandle *handle = new Vizkit3dPluginHandle(pName.toStdString());
                    handle->factory = factory;
                    handle->libararyName = libPath;

                    handle->typeName = parameterList[0].data();
                    //getPluginsForType expects type names without prefix "::". moc keeps them as given.
                    handle->typeName = boost::regex_replace(handle->typeName, boost::regex("^::"), "");
                    handle->method = method;
                    typeMap[handle->typeName] = handle;
                }
            }      

            if (!foundUpdateFunctions)
            {
                LOG_WARN_S << "Vizkit3dPluginRepository: no update functions found in " << libPath << "..";
                Vizkit3dPluginHandle *handle = new Vizkit3dPluginHandle(pName.toStdString());
                handle->factory = factory;
                handle->libararyName = libPath;
                handle->typeName = std::string();
                handle->method = QMetaMethod();

                datalessPlugins.push_back(handle);
            }
            
            delete plugin;
            
            for(const auto &it : typeMap)
            {                
                typeToPlugins[it.second->typeName].push_back(it.second);
            }
        }

        delete availablePlugins;
    }
}

const std::vector< Vizkit3dPluginHandle *>& Vizkit3dPluginRepository::getPluginsForType(const std::string& type, const Typelib::Registry* registry)
{
    if(type.empty())
        return empty;

    //try for a direct match first
    std::string dottedType = convertTypelibToDottedTypeName(type);
 
    auto it = typeToPlugins.find(dottedType);
    if (it != typeToPlugins.end())
    {
        return it->second;
    }

    if(!registry)
    {
        return empty;
    }

    //if there is a typelib registry, try using its isSame method
    auto typelibType = registry->get(type);
    if(!typelibType)
    {
        LOG_WARN_S << "Vizkit3dPluginRepository: could not find typelib type for " << type << " ..";
        return empty;
    }

    std::vector< Vizkit3dPluginHandle *> resultset;
    for(const auto &h: typeToPlugins)
    {
        std::string pluginTypeName = convertDottedToTypelibTypeName(h.first);

        auto pluginType = registry->get(pluginTypeName);
        if (!pluginType)
        {
            //these are normal, the types for these plugins are not used in this deployment.
            //no harm either, we do not remember a negative result from this.
            //the scenario for a failure here would be: one registry is used for the sample and lookup here,
            //that does not contain the typedef pluginTypeName refers to.
            //another registry has the typedef, but now the basename already has typeToPlugins populated and
            //the (now possible) resolution of the typedef is not found.
            LOG_DEBUG_S << "Vizkit3dPluginRepository: could not find typelib type for " << pluginTypeName << " ..";
        }

        if (pluginType && typelibType->isSame(*pluginType))
        {
            resultset.insert(resultset.end(), h.second.begin(), h.second.end());
        }
    }

    //if the result depends on the used registry, typeToPlugins should not be populated,
    //but that seems unlikely
    if(resultset.empty())
    {
        typeToPlugins[dottedType] = empty;
    }
    else
    {
        typeToPlugins[dottedType] = resultset;
    }

    return typeToPlugins[dottedType];
}

const std::vector<Vizkit3dPluginHandle*> Vizkit3dPluginRepository::getAllAvailablePlugins()
{
    std::vector<Vizkit3dPluginHandle*> plugins;
    for (auto &t2p: typeToPlugins)
    {
        for (auto &plugin: t2p.second)
        {
            plugins.push_back(plugin);
        }
    }
    for (auto &dlp : datalessPlugins)
    {
        plugins.push_back(dlp);
    }

    return plugins;
}

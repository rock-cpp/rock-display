#include "Vizkit3dPluginRepository.hpp"
#include <base-logging/Logging.hpp>
#include <iostream>
#include <boost/regex.hpp>

VizHandle *Vizkit3dPluginHandle::createViz() const
{
    Vizkit3dVizHandle *newHandle = new Vizkit3dVizHandle;
    newHandle->plugin = factory->createPlugin(QString::fromStdString(pluginName));
    newHandle->method = method;
    return newHandle;
}

void Vizkit3dVizHandle::updateVisualizer(RTT::base::DataSourceBase::shared_ptr data)
{
    QGenericArgument val("void *", data.get()->getRawConstPointer());
    if (!val.data())
    {
        return;
    }

    method.invoke(plugin, val);
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
        
        Vizkit3dPluginHandle handle;
        handle.factory = factory;
        handle.libararyName = libPath;
        
        QStringList *availablePlugins = factory->getAvailablePlugins();
                
        for(const QString &pName: *availablePlugins)
        {   
            std::map<std::string, Vizkit3dPluginHandle> typeMap;
            handle.pluginName = pName.toStdString();
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
                    handle.typeName = parameterList[0].data();
                    //getPluginsForType expects type names without prefix "::". moc keeps them as given.
                    handle.typeName = boost::regex_replace(handle.typeName, boost::regex("^::"), "");
                    handle.method = method;
                    typeMap[handle.typeName] = handle;
                }
            }      

            if (!foundUpdateFunctions)
            {
                LOG_WARN_S << "Vizkit3dPluginRepository: no update functions found in " << libPath << "..";
            }
            
            delete plugin;
            
            for(const auto &it : typeMap)
            {                
                handle = it.second;
                typeToPlugins[it.second.typeName].push_back(it.second);
            }
        }

        delete availablePlugins;
    }
}

const std::vector< Vizkit3dPluginHandle >& Vizkit3dPluginRepository::getPluginsForType(const std::string& type, const Typelib::Registry* registry)
{
    if(type.empty())
        return empty;

    //try for a direct match first
    std::string dottedType = type.substr(1, type.size());
    dottedType = boost::regex_replace(dottedType, boost::regex("_m"), "");
    dottedType = boost::regex_replace(dottedType, boost::regex("</"), "<");
    dottedType = boost::regex_replace(dottedType, boost::regex("/"), "::");
    dottedType = boost::regex_replace(dottedType, boost::regex("\\s\\[.*\\]"), "");
    dottedType = boost::regex_replace(dottedType, boost::regex(">>"), "> >");
 
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

    std::vector< Vizkit3dPluginHandle > resultset;
    for(const auto &h: typeToPlugins)
    {
        std::string pluginTypeName = h.first;
        pluginTypeName = boost::regex_replace(pluginTypeName, boost::regex("> >"), ">>");
        pluginTypeName = boost::regex_replace(pluginTypeName, boost::regex("::"), "/");
        pluginTypeName = boost::regex_replace(pluginTypeName, boost::regex("<"), "</");
        pluginTypeName = std::string("/")+pluginTypeName;

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
            plugins.push_back(&plugin);
        }
    }
    
    return plugins;
}
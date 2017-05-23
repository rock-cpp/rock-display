#include "Vizkit3dPluginRepository.hpp"
#include <base-logging/Logging.hpp>

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
        
        PluginHandle handle;
        handle.factory = factory;
        handle.libararyName = libPath;
        
        QStringList *availablePlugins = factory->getAvailablePlugins();
                
        for(const QString &pName: *availablePlugins)
        {   
            std::map<std::string, PluginHandle> typeMap;
            handle.pluginName = pName.toStdString();
            QObject *plugin = factory->createPlugin(pName);
            LOG_INFO_S << "plugin " << pName.toStdString();
            
            const QMetaObject *metaPlugin = plugin->metaObject();
        
            for(int i = 0 ; i < metaPlugin->methodCount(); i++)
            {
                QMetaMethod method = metaPlugin->method(i);
                auto parameterList = method.parameterTypes();
                if(parameterList.size() != 1)
                    continue;
                
                std::string signature = method.signature();
                std::string update("update");
                
                if(signature.size() > update.size() && signature.substr(0, update.size()) == update)
                {
                    handle.typeName = parameterList[0].data();
                    handle.method = method;
                    typeMap[handle.typeName] = handle;
                }
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

VizHandle Vizkit3dPluginRepository::getNewVizHandle(const PluginHandle& handle)
{
    VizHandle newHandle;
    newHandle.plugin = handle.factory->createPlugin(handle.pluginName.c_str());
    newHandle.method = handle.method;
    return newHandle;
}

#include <boost/regex.hpp>
const std::vector< PluginHandle >& Vizkit3dPluginRepository::getPluginsForType(const std::string& type)
{
    if(type.empty())
        return empty;

    std::string dottedType = type.substr(1, type.size());
    dottedType = boost::regex_replace(dottedType, boost::regex("_m"), "");
    dottedType = boost::regex_replace(dottedType, boost::regex("</"), "<");
    dottedType = boost::regex_replace(dottedType, boost::regex("/"), "::");
    dottedType = boost::regex_replace(dottedType, boost::regex("\\s\\[.*\\]"), "");
    dottedType = boost::regex_replace(dottedType, boost::regex(">>"), "> >");
 
    for(const auto &h: typeToPlugins)
    {
        if(h.first == dottedType)
        {
            return h.second;
        }
    }
    
    auto it = typeToPlugins.find(dottedType);
    
    if (it == typeToPlugins.end())
    {
       return empty;
    }

    return it->second;    
}

const std::vector<PluginHandle> Vizkit3dPluginRepository::getAllAvailablePlugins()
{
    std::vector<PluginHandle> plugins;
    for (auto &t2p: typeToPlugins)
    {
        for (auto &plugin: t2p.second)
        {
            plugins.push_back(plugin);
        }
    }
    
    return plugins;
}
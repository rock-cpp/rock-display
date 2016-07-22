#include "Vizkit3dPluginRepository.hpp"

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
            std::cout << "Fail " << std::endl;
        }
        vizkit3d::VizkitPluginFactory *factory = dynamic_cast<vizkit3d::VizkitPluginFactory *>(loader.instance());
        if(!factory)
        {
            std::cout << "Internal error, plugin is not a factory" << std::endl;
            continue;
        }
        
        PluginHandle handle;
        handle.factory = factory;
        handle.libararyName = libPath;
        
        QStringList *availablePlugins = factory->getAvailablePlugins();
        
        for(const QString &pName: *availablePlugins)
        {
            std::cout << "Plugins : " << pName.toStdString() << std::endl;
        }        
        for(const QString &pName: *availablePlugins)
        {
            std::map<std::string, PluginHandle> typeMap;
            handle.pluginName = pName.toStdString();
            QObject *plugin = factory->createPlugin(pName);
            
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
//                     std::cout << "Into typeMap " << handle.typeName << " " << handle.libararyName << " " << handle.pluginName << " " << std::endl;
                    typeMap[handle.typeName] = handle;
                }
            }
            
            
            delete plugin;
            std::cout << "Cur Plugin " << pName.toStdString() << std::endl;
            
            for(const auto &it : typeMap)
            {                
                handle = it.second;
                std::cout << "Regisering handle " << handle.typeName << " " << handle.libararyName << " " << handle.pluginName << " " << std::endl;
                typeToPlugins[it.second.typeName].push_back(it.second);
            }
            std::cout << std::endl;
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
    dottedType = boost::regex_replace(dottedType, boost::regex("/"), "::");
    
    std::cout << "Dttet type '" << dottedType << "'" << std::endl;
 
    for(const auto &h: typeToPlugins)
    {
        std::cout << "Type '" << h.first << "' handles " << h.second.size() << std::endl;
        if(h.first == dottedType)
        {
            std::cout << "FOUND MATCH" << std::endl;
            return h.second;
        }
    }
    
    auto it = typeToPlugins.find(dottedType);
    
    if(it == typeToPlugins.end());
       return empty;

    return it->second;    
}



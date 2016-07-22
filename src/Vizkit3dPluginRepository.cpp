#include "Vizkit3dPluginRepository.hpp"

Vizkit3dPluginRepository::Vizkit3dPluginRepository(QStringList &plugins)
{
    QPluginLoader loader;
    
    for(QString &pluginString : plugins)
    {
        loader.setFileName(pluginString.split('@').back());
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
        handle.pluginName = pluginString.split('@').front().toStdString();
        
        QStringList *availablePlugins = factory->getAvailablePlugins();
        
        std::map<std::string, bool> typeMap;
        
        for(const QString &pName: *availablePlugins)
        {
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
                    typeMap[(parameterList.first().data())] = true;
                }
            }
        }
        
//         std::cout << "Plugin " << pluginString.toStdString() << "  supports following types " << std::endl;
        for(auto it : typeMap)
        {
            handle.typeName = it.first;
//             std::cout << it.first << std::endl;
            typeToPlugins[it.first].push_back(handle);
        }
        
        delete availablePlugins;
    }
}

const std::vector< PluginHandle >& Vizkit3dPluginRepository::getPluginsForType(const std::string& type)
{
    auto it = typeToPlugins.find(type);
    
    if(it == typeToPlugins.end());
       return empty;

    return it->second;    
}



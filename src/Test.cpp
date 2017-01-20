#include <iostream>
#include <rtt/transports/corba/ApplicationServer.hpp>
#include <rtt/types/TypeInfoRepository.hpp>
#include <vizkit3d/MotionCommandVisualization.hpp>

#include "TaskModel.hpp"
#include "Mainwindow.hpp"
#include <vizkit3d/Vizkit3DWidget.hpp>
#include "Vizkit3dPluginRepository.hpp"
#include <QApplication>
#include <QTimer>

#include <orocos_cpp/TypeRegistry.hpp>
#include <orocos_cpp/PluginHelper.hpp>

#include <base/commands/Motion2D.hpp>
#include <base/Time.hpp>
#include <base/samples/RigidBodyState.hpp>

orocos_cpp::TypeRegistry typeReg;

bool loadTypkekit(const std::string &typeName)
{
    std::cout << "Type " << typeName << " requested " << std::endl;
    std::string tkName;
    if(typeReg.getTypekitDefiningType(typeName, tkName))
    {
        std::cout << "TK " << tkName << " is defining the type " << typeName << std::endl;
        if(orocos_cpp::PluginHelper::loadTypekitAndTransports(tkName))
        {
            std::cout << "TK loaded" << std::endl;
            return true;
        }
    }
    return false;
}

int main(int argc, char** argv)
{
    typeReg.loadTypelist();
    RTT::corba::ApplicationServer::InitOrb(argc, argv);

    QApplication app(argc, argv);

    RTT::types::TypeInfoRepository *ti = RTT::types::TypeInfoRepository::Instance().get();
    boost::function<bool (const std::string &)> f(&loadTypkekit);
    ti->setAutoLoader(f);

    vizkit3d::Vizkit3DWidget vizkit3dWidget;
    QStringList *plugins = vizkit3dWidget.getAvailablePlugins();

    for(QString &p : *plugins)
    {
        std::cout << "Plugin : " << p.toStdString() << std::endl;
    }

    Vizkit3dPluginRepository repo(*plugins);

    const std::vector<PluginHandle> handles = repo.getPluginsForType("/base/commands/Motion2D");
    if (handles.empty())
    {
      throw std::runtime_error("no plugin for type available..");
    }
    
    const std::vector<PluginHandle> handles2 = repo.getPluginsForType("/base/samples/RigidBodyState");
    if (handles2.empty())
    {
      throw std::runtime_error("no plugin for type available..");
    }
    
    VizHandle vizHandle = repo.getNewVizHandle(handles.front());
    vizkit3d::Vizkit3DWidget widget3d;
    widget3d.addPlugin(vizHandle.plugin);
    widget3d.show();
    
    double lowerBound = 0;
    double upperBound = 1;
    std::uniform_real_distribution<double> unif(lowerBound, upperBound);
    std::default_random_engine re;
    
    base::Time now = base::Time::now();
    
    while (true)
    { 
      if ((base::Time::now() - now).toMilliseconds() > 1000)
      { 
        base::samples::RigidBodyState *rbs = new base::samples::RigidBodyState();
        rbs->position = base::Position(0.5, 1, 0.2);
        QGenericArgument data2("base::samples::RigidBodyState", (const void *) rbs);
        vizHandle.method.invoke(vizHandle.plugin, data2);
        
	std::cout << "update.." << std::endl;
	now = base::Time::now();
      }
      
      app.processEvents();
    }
}
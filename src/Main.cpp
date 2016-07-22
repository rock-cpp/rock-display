#include <iostream>
#include <rtt/transports/corba/ApplicationServer.hpp>
#include <rtt/types/TypeInfoRepository.hpp>

#include <QApplication>
#include <QTimer>
#include "TaskModel.hpp"
#include "Mainwindow.hpp"
#include <vizkit3d/Vizkit3DWidget.hpp>
#include "Vizkit3dPluginRepository.hpp"

#include <orocos_cpp/TypeRegistry.hpp>
#include <orocos_cpp/PluginHelper.hpp>

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

    MainWindow w;
    w.show();

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
    
    QTimer timer;
    timer.setInterval(100);

    QObject::connect(&timer, SIGNAL(timeout()), &w, SLOT(queryTasks()));
    timer.start();

    return app.exec();
}

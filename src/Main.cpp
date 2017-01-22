#include <iostream>
#include <rtt/transports/corba/ApplicationServer.hpp>
#include <rtt/types/TypeInfoRepository.hpp>

#include "TaskModel.hpp"
#include "Mainwindow.hpp"
#include <vizkit3d/Vizkit3DWidget.hpp>
#include "Vizkit3dPluginRepository.hpp"
#include <QApplication>
#include <QTimer>

#include <orocos_cpp/TypeRegistry.hpp>
#include <orocos_cpp/PluginHelper.hpp>
#include <thread>

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
    
    while (true)
    {
        w.updateTaskItems();
        w.update();
        w.repaint();
        w.widget3d.update();
        app.processEvents();
    }
}
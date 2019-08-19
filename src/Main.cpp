#include <iostream>
#include <rtt/transports/corba/ApplicationServer.hpp>
#include <rtt/types/TypeInfoRepository.hpp>
#include <signal.h>

#include "TaskModel.hpp"
#include "Mainwindow.hpp"
#include <vizkit3d/Vizkit3DWidget.hpp>
#include "Vizkit3dPluginRepository.hpp"
#include <QApplication>
#include <QTimer>

#include <orocos_cpp/TypeRegistry.hpp>
#include <orocos_cpp/PluginHelper.hpp>
#include <thread>

#include <base-logging/Logging.hpp>

orocos_cpp::TypeRegistry typeReg;

bool loadTypkekit(const std::string &typeName)
{
    LOG_INFO_S << "Loading typekit for " << typeName;
    std::string tkName;
    if(typeReg.getTypekitDefiningType(typeName, tkName))
    {
        LOG_INFO_S << "Typekit name: " << tkName;
        if (orocos_cpp::PluginHelper::loadTypekitAndTransports(tkName))
        {
            return true;
        }
    }
    LOG_WARN_S << "failed to load typekit for " << typeName;
    return false;
}

int main(int argc, char** argv)
{
    typeReg.loadTypeRegistries();
    RTT::corba::ApplicationServer::InitOrb(argc, argv);

    QApplication app(argc, argv);
    MainWindow w;
    w.show();

    RTT::types::TypeInfoRepository *ti = RTT::types::TypeInfoRepository::Instance().get();
    boost::function<bool (const std::string &)> f(&loadTypkekit);
    ti->setAutoLoader(f);

    return app.exec();
}
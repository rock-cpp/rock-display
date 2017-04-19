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
MainWindow *w;

bool loadTypkekit(const std::string &typeName)
{
    LOG_INFO_S << "Loading typekit for " << typeName;
    std::string tkName;
    if(typeReg.getTypekitDefiningType(typeName, tkName))
    {
        LOG_INFO_S << "Typekit name: " << tkName;
        if(orocos_cpp::PluginHelper::loadTypekitAndTransports(tkName))
        {
            return true;
        }
    }
    LOG_WARN_S << "failed to load typekit for " << typeName;
    return false;
}

void handleSigInt(int v)
{
    delete w;
    QApplication::quit();
    exit(0);
}

int main(int argc, char** argv)
{
    typeReg.loadTypelist();
    RTT::corba::ApplicationServer::InitOrb(argc, argv);

    QApplication app(argc, argv);

    w = new MainWindow();
    w->show();

    RTT::types::TypeInfoRepository *ti = RTT::types::TypeInfoRepository::Instance().get();
    boost::function<bool (const std::string &)> f(&loadTypkekit);
    ti->setAutoLoader(f);
    
    QTimer timer;
    timer.setInterval(100);
    
    QObject::connect(&timer, SIGNAL(timeout()), w, SLOT(updateTasks()), Qt::QueuedConnection);
    timer.start();
    
    struct sigaction mainWindowSigIntHandler;
    mainWindowSigIntHandler.sa_handler = handleSigInt;
    sigemptyset(&mainWindowSigIntHandler.sa_mask);
    mainWindowSigIntHandler.sa_flags = 0;
    sigaction(SIGTERM, &mainWindowSigIntHandler, NULL);
    sigaction(SIGINT, &mainWindowSigIntHandler, NULL);
    
    return app.exec();
}

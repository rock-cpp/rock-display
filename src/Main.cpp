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

orocos_cpp::TypeRegistry typeReg;
MainWindow *w;

class RockDisplay : public QApplication
{
public:
    RockDisplay(int& argc, char** argv, int = ApplicationFlags)
        : QApplication(argc, argv)
    {   
    }
    
    bool notify(QObject *receiver, QEvent *event)
    {
        try
        {
            return QApplication::notify(receiver, event);
        }
        catch (std::exception& e)
        {
            std::cout << "excpetion in QApplication::notify: " << e.what() << std::endl;
            return false;
        }
    }
};

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

    RockDisplay app(argc, argv);

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
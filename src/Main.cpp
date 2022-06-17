#include <iostream>
#include <orocos_cpp/orocos_cpp.hpp>
#include <rtt/transports/corba/ApplicationServer.hpp>
#include <rtt/types/TypeInfoRepository.hpp>
#include <signal.h>

#include "TaskModel.hpp"
#include "Mainwindow.hpp"
#include <vizkit3d/Vizkit3DWidget.hpp>
#include "Vizkit3dPluginRepository.hpp"
#include <QApplication>
#include <QTimer>

#include <thread>

#include <base-logging/Logging.hpp>


int main(int argc, char** argv)
{
    orocos_cpp::OrocosCppConfig config;
    orocos_cpp::OrocosCpp orocos;

    config.load_all_packages = true;
    orocos.initialize(config);

    base::logging::Logger::getInstance()->configure(base::logging::DEBUG, stdout);

    QApplication app(argc, argv);
    MainWindow w;
    w.show();

    return app.exec();
}
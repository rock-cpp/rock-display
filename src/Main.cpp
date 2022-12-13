#include <orocos_cpp/orocos_cpp.hpp>

#include "Mainwindow.hpp"
#include <QApplication>

#include <base-logging/Logging.hpp>


int main(int argc, char** argv)
{
    orocos_cpp::OrocosCppConfig config;
    orocos_cpp::OrocosCpp orocos;

    config.load_all_packages = true;
    orocos.initialize(config);

    base::logging::Logger::getInstance()->configure(base::logging::DEBUG, stdout);

    QApplication app(argc, argv);
    MainWindow w(orocos);
    w.show();

    return app.exec();
}
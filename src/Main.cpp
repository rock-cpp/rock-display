#include <iostream>
#include <rtt/transports/corba/ApplicationServer.hpp>
#include <rtt/types/TypeInfoRepository.hpp>

#include "ui_task_inspector_window.h"
#include <QApplication>
#include <QTimer>
#include "TaskModel.hpp"
#include "RDItemDelegate.hpp"

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

            std::cout << "Tk loaded" << std::endl;
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

    QMainWindow mainWindow;

    Ui::MainWindow ui;
    ui.setupUi(&mainWindow);

    mainWindow.show();
    TaskModel *model = new TaskModel();
    ui.treeView->setModel(model);

    //
    RDItemDelegate rdid(ui.treeView);
    ui.treeView->setItemDelegate(&rdid);

    RTT::types::TypeInfoRepository *ti = RTT::types::TypeInfoRepository::Instance().get();
    boost::function<bool (const std::string &)> f(&loadTypkekit);
    ti->setAutoLoader(f);

    QTimer timer;
    timer.setInterval(100);

    QObject::connect(&timer, SIGNAL(timeout()), model, SLOT(queryTasks()));
    timer.start();

    return app.exec();
}

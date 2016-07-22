#include <rtt/TaskContext.hpp>
#include "Mainwindow.hpp"
#include "ui_task_inspector_window.h"
#include "Types.hpp"
#include "TypedItem.hpp"
#include <QCursor>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    view = ui->treeView;
    view->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(view, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(prepareMenu(QPoint)));

    model = new TaskModel(this);

    view->setModel(model);

    auto *list = widget3d.getAvailablePlugins();
    pluginRepo = new Vizkit3dPluginRepository(*list);
    delete list;
}

MainWindow::~MainWindow()
{
    delete ui;
    delete pluginRepo;
}

void MainWindow::prepareMenu(const QPoint & pos)
{
    QModelIndex mi = view->indexAt(pos);
    QStandardItem *item = model->itemFromIndex(mi);
    QMenu menu(this);

    if (TypedItem *ti = dynamic_cast<TypedItem*>(item))
    {
        switch (ti->type()) {
            case ItemType::TASK:
                {

                QAction *act = menu.addAction("Activate");
                QAction *sta = menu.addAction("Start");
                QAction *sto = menu.addAction("Stop");
                QAction *con = menu.addAction("Configure");

                connect(act, SIGNAL(triggered()), this, SLOT(activateTask()));
                connect(sta, SIGNAL(triggered()), this, SLOT(startTask()));
                connect(sto, SIGNAL(triggered()), this, SLOT(stopTask()));
                connect(con, SIGNAL(triggered()), this, SLOT(configureTask()));
                }
                break;
            case ItemType::OUTPUTPORT:
            {
                // for (b : bla) {}
                OutputPortItem *outPort = static_cast<OutputPortItem *>( ti->getData());
                std::cout << "Type of port is " << outPort->getType() << std::endl;
                const auto &handles = pluginRepo->getPluginsForType(outPort->getType()); 
                
                std::cout << "Got " << handles.size() << " handles " << std::endl;
                
                for(const PluginHandle &handle: handles)
                {
                    menu.addAction(handle.pluginName.c_str());
                }
                
            }
                break;
            default:
                printf("Falscher Typ %d\n", ti->type());
        }

        QPoint pt(pos);
        menu.exec(QCursor::pos());
    } else {
        printf("Cast kaputt... Type: %d\n", item->type()); //TODO remove after testing
    }
}

void MainWindow::queryTasks()
{
    model->queryTasks();
}

void MainWindow::activateTask()
{

}

void MainWindow::startTask()
{
    //task->start();
}

void MainWindow::stopTask()
{
    //task->stopTask();
}

void MainWindow::configureTask()
{
    //task->configure();
}

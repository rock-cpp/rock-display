#include <rtt/transports/corba/TaskContextProxy.hpp>
#include "Mainwindow.hpp"
#include "ui_task_inspector_window.h"
#include "Types.hpp"
#include "TypedItem.hpp"
#include <QCursor>

#include <base/commands/Motion2D.hpp>

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
    
    modelUpdater = new std::thread([&] { while (true) { this->queryTasks(); usleep(100); }; });
}

MainWindow::~MainWindow()
{
    delete ui;
    delete pluginRepo;
}

void MainWindow::prepareMenu(const QPoint & pos)
{
//     std::cout << "prepare menu.." << std::endl;
    QModelIndex mi = view->indexAt(pos);
    QStandardItem *item = model->itemFromIndex(mi);
    QMenu menu(this);

    if (TypedItem *ti = dynamic_cast<TypedItem*>(item))
    {
        switch (ti->type()) {
            case ItemType::TASK:
            {
                TaskItem *titem = dynamic_cast<TaskItem*>(ti);

                task = titem->getTaskContext();

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
                OutputPortItem *outPort = static_cast<OutputPortItem *>( ti->getData());
//                 std::cout << "Type of port is " << outPort->getType() << std::endl;
                const auto &handles = pluginRepo->getPluginsForType(outPort->getType());

//                 std::cout << "Got " << handles.size() << " handles " << std::endl;

                for (const PluginHandle &handle : handles)
                {
                    QSignalMapper* signalMapper = new QSignalMapper (this) ;
                    QAction *act = menu.addAction(handle.pluginName.c_str());

                    connect(act, SIGNAL(triggered()), signalMapper, SLOT(map()));

                    signalMapper->setMapping(act, new DataContainer(handle, outPort));

                    connect(signalMapper, SIGNAL(mapped(QObject*)), this, SLOT(handleOutputPort(QObject*)));
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

void MainWindow::handleOutputPort(QObject *obj)
{
    DataContainer *d = static_cast<DataContainer*>(obj);
    OutputPortItem *it = d->getOutputPortItem();
    PluginHandle ph = d->getPluginHandle();

    VizHandle nh = pluginRepo->getNewVizHandle(ph);
    widget3d.addPlugin(nh.plugin);
    widget3d.show();
    
    std::cout << "nh: " << nh.plugin << std::endl;

    it->addPlugin(nh);
}

void MainWindow::queryTasks()
{
    model->queryTasks();
}

void MainWindow::activateTask()
{
    task->activate();
}

void MainWindow::startTask()
{
    task->start();
}

void MainWindow::stopTask()
{
    task->stop();
}

void MainWindow::configureTask()
{
    task->configure();
}

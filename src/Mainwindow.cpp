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
    connect(view, SIGNAL(expanded(QModelIndex)), this, SLOT(onExpanded(QModelIndex)));
    connect(view, SIGNAL(collapsed(QModelIndex)), this, SLOT(onCollapsed(QModelIndex)));
    
    view->setSortingEnabled(true);
    
    model = new TaskModel(this);

    view->setModel(model);
    connect(this, SIGNAL(stopNotifier()), model->notifier, SLOT(stopNotifier()));

    auto *list = widget3d.getAvailablePlugins();
    pluginRepo = new Vizkit3dPluginRepository(*list);
    delete list;
    
    model->notifier->start();
}

MainWindow::~MainWindow()
{
    emit stopNotifier();
    model->notifier->quit();

    if (model->notifier->wait())
    {
        model->notifier->terminate();
        model->notifier->wait();
    }
    
    delete model->notifier;
    delete model;
    delete ui;
    delete pluginRepo;
}

void MainWindow::onExpanded(const QModelIndex& index)
{
    this->setItemExpanded(index, true);
}

void MainWindow::onCollapsed(const QModelIndex& index)
{
    this->setItemExpanded(index, false);
}

void MainWindow::setItemExpanded(const QModelIndex& index, bool expanded)
{
    QStandardItem *item = model->itemFromIndex(index);
    
    if (TypedItem *ti = dynamic_cast<TypedItem*>(item))
    {
        if (ti->type() == ItemType::TASK)
        {
            TaskItem *titem = static_cast<TaskItem*>(ti->getData());
            
            for (std::map<std::string, PortItem *>::iterator portIt = titem->getPorts().begin(); portIt != titem->getPorts().end(); portIt++)
            {
                portIt->second->setExpanded(expanded);
            }
        }
    }
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
                TaskItem *titem = static_cast<TaskItem*>(ti->getData());

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
                const auto &handles = pluginRepo->getPluginsForType(outPort->getType());

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

void MainWindow::updateTaskItems()
{
    model->updateTaskItems();
}
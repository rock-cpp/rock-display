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
    
    model = new NameServiceModel(this);
    connect(this, SIGNAL(stopNotifier()), model, SLOT(stop()));
    
    TaskModel *initialTasks = new TaskModel();
    model->addTaskModel(initialTasks);

    qRegisterMetaType<std::string>("std::string");
    view->setModel(model);

    auto *list = widget3d.getAvailablePlugins();
    pluginRepo = new Vizkit3dPluginRepository(*list);
    delete list;
    
    std::vector<PluginHandle> allAvailablePlugins = pluginRepo->getAllAvailablePlugins();
    for (const PluginHandle &handle: allAvailablePlugins)
    {
        QSignalMapper* signalMapper = new QSignalMapper (this) ;
        QAction *act = ui->menuWidgets->addAction(handle.pluginName.c_str());

        connect(act, SIGNAL(triggered()), signalMapper, SLOT(map()));

        signalMapper->setMapping(act, new DataContainer(handle, nullptr));

        connect(signalMapper, SIGNAL(mapped(QObject*)), this, SLOT(openPlugin(QObject*)));
    }
    
    connect(ui->actionAdd_name_service, SIGNAL(triggered()), this, SLOT(addNameService()));
    
    
    nameServiceDialog = new AddNameServiceDialog();
    connect(nameServiceDialog, SIGNAL(requestNameServiceAdd(const std::string &)), model, SLOT(addNameService(const std::string &)));
    initialTasks->notifierThread->start();
}

AddNameServiceDialog::AddNameServiceDialog(QWidget* parent): QDialog(parent)
{
    nameServiceIP = new QLineEdit();
    
    QPushButton *addBtn = new QPushButton(tr("Add"));
    addBtn->setDefault(true);
    
    QPushButton *cancelBtn = new QPushButton(tr("Cancel"));
    
    QDialogButtonBox *btnBox = new QDialogButtonBox(Qt::Horizontal);
    btnBox->addButton(addBtn, QDialogButtonBox::AcceptRole);
    btnBox->addButton(cancelBtn, QDialogButtonBox::RejectRole);

    connect(btnBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(btnBox, SIGNAL(rejected()), this, SLOT(reject()));
    
    QVBoxLayout *lt = new QVBoxLayout;
    lt->addWidget(nameServiceIP);
    lt->addWidget(btnBox);

    setLayout(lt);
}

void AddNameServiceDialog::addNameService()
{
    std::string nameServiceIPStr = nameServiceIP->text().toStdString();
    
    if (!nameServiceIPStr.empty())
    {
        emit requestNameServiceAdd(nameServiceIPStr);
    }
}

void AddNameServiceDialog::accept()
{
    QDialog::accept();
    addNameService();
}

void MainWindow::addNameService()
{
    nameServiceDialog->show();
    nameServiceDialog->exec();
}

void MainWindow::openPlugin(QObject *obj)
{
    DataContainer *d = static_cast<DataContainer*>(obj);
    PluginHandle ph = d->getPluginHandle();

    VizHandle nh = pluginRepo->getNewVizHandle(ph);
    widget3d.addPlugin(nh.plugin);
    widget3d.show();
}

MainWindow::~MainWindow()
{
    emit stopNotifier();
    
    model->waitForTerminate();

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
                
            case ItemType::CONFIGITEM:
            {
                ItemBase *titem = static_cast<ItemBase*>(ti->getData());
                std::string name = titem->getRow().first()->text().toStdString();
                std::string typeName = titem->getRow().last()->text().toStdString();
                std::size_t start_pos = 0;
                start_pos = typeName.find("_m", start_pos);
                if (start_pos != std::string::npos)
                {
                    typeName.replace(start_pos, 2, "");
                }

                const auto &handles = pluginRepo->getPluginsForType(typeName);

                for (const PluginHandle &handle : handles)
                {
                    QSignalMapper* signalMapper = new QSignalMapper (this) ;
                    QAction *act = menu.addAction(handle.pluginName.c_str());

                    connect(act, SIGNAL(triggered()), signalMapper, SLOT(map()));

                    signalMapper->setMapping(act, new DataContainer(handle, titem));

                    connect(signalMapper, SIGNAL(mapped(QObject*)), this, SLOT(handleOutputPort(QObject*)));
                }

            }
                break;
            default:
                printf("wrong type %d\n", ti->type());
        }

        QPoint pt(pos);
        menu.exec(QCursor::pos());
    }
}

void MainWindow::handleOutputPort(QObject *obj)
{
    DataContainer *d = static_cast<DataContainer*>(obj);
    ItemBase *it = d->getItem();
    PluginHandle ph = d->getPluginHandle();

    VizHandle nh = pluginRepo->getNewVizHandle(ph);
    widget3d.addPlugin(nh.plugin);
    widget3d.show();

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

void MainWindow::updateTasks()
{
    model->updateTasks();
}
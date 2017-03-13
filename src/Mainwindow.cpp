#include <rtt/transports/corba/TaskContextProxy.hpp>
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
    connect(view, SIGNAL(expanded(QModelIndex)), this, SLOT(onExpanded(QModelIndex)));
    connect(view, SIGNAL(collapsed(QModelIndex)), this, SLOT(onCollapsed(QModelIndex)));
    
    QMenu *pluginManager = new QMenu("Manage Visualizers");
    QAction *removeAllPluginsAction = pluginManager->addAction("remove all active visualizers");
    connect(removeAllPluginsAction, SIGNAL(triggered()), this, SLOT(removeAllPlugins()));
    
    ui->menubar->addMenu(pluginManager);
    
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

void MainWindow::removeAllPlugins()
{
    while (!activePlugins.empty())
    {
        auto &plugin = activePlugins.front();
        removePlugin(plugin.first, plugin.second);
    }
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
    addPlugin(ph, nullptr);
}

void MainWindow::cleanup()
{
    emit stopNotifier();
    
    model->waitForTerminate();

    delete model;
    delete ui;
    delete pluginRepo;
}

MainWindow::~MainWindow()
{
    cleanup();
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
        if (ti->type() == ItemType::CONFIGITEM)
        {
            ItemBase *titem = static_cast<ItemBase *>(ti->getData());
            titem->setExpanded(expanded);
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
            case ItemType::OUTPUTPORT:
            {
                std::string typeName = "";
                ItemBase *titem = nullptr;
                OutputPortItem *outport = nullptr;
                
                if (ti->type() == ItemType::CONFIGITEM)
                {
                    titem = static_cast<ItemBase*>(ti->getData());
                    typeName = titem->getRow().last()->text().toStdString();
                }
                else
                {
                    outport = static_cast<OutputPortItem*>(ti->getData());
                    typeName = outport->getType();
                }

                const auto &handles = pluginRepo->getPluginsForType(typeName);

                for (const PluginHandle &handle : handles)
                {
                    QSignalMapper* signalMapper = new QSignalMapper(this);
                    QAction *act = nullptr;
                    
                    if (ti->type() == ItemType::CONFIGITEM)
                    {
                        if (titem->hasVizualizer(handle.pluginName))
                        {
                            act = menu.addAction(std::string(std::string("remove ") + handle.pluginName).c_str());
                        }
                        else
                        {
                            act = menu.addAction(handle.pluginName.c_str());
                        }
                    }
                    else
                    {
                        if (outport->waitingVizualizer.find(handle.pluginName) != outport->waitingVizualizer.end())
                        {
                            act = menu.addAction(std::string(std::string("remove ") + handle.pluginName).c_str());
                        }
                        else
                        {
                            act = menu.addAction(handle.pluginName.c_str());
                        }
                    }

                    connect(act, SIGNAL(triggered()), signalMapper, SLOT(map()));
                    signalMapper->setMapping(act, new DataContainer(handle, ti));

                    connect(signalMapper, SIGNAL(mapped(QObject*)), this, SLOT(handleOutputPort(QObject*)));
                }
            }
                break;
            default:
                return;
        }

        
        
        QPoint pt(pos);
        menu.exec(QCursor::pos());
    }
}

void MainWindow::addPlugin(PluginHandle &ph, TypedItem* ti)
{
    VizHandle nh = pluginRepo->getNewVizHandle(ph);
    widget3d.addPlugin(nh.plugin);
    activePlugins.push_back(std::make_pair(nh.plugin, ti));
    
    if (ti)
    {
        if (ti->type() == ItemType::CONFIGITEM)
        {
            ItemBase *item = static_cast<ItemBase *>(ti->getData());
            item->addPlugin(std::make_pair(ph.pluginName, nh));
        }
        else if (ti->type() == ItemType::OUTPUTPORT)
        {
            OutputPortItem *outputPort = static_cast<OutputPortItem *>(ti->getData());
            outputPort->addPlugin(std::make_pair(ph.pluginName, nh));
        }
    }
    
    widget3d.show();
}

void MainWindow::removePlugin(QObject *plugin, TypedItem *ti)
{
    widget3d.removePlugin(plugin);
    
    if (ti)
    {
        if (ti->type() == ItemType::CONFIGITEM)
        {
            ItemBase *item = static_cast<ItemBase *>(ti->getData());
            item->removeVizualizer(plugin);
        }
        else if (ti->type() == ItemType::OUTPUTPORT)
        {
            OutputPortItem *outport = static_cast<OutputPortItem *>(ti->getData());
            if (!outport->removeVizualizer(plugin))
            {
                outport->getItemBase()->removeVizualizer(plugin);
            }
        }
    }
    
    for (std::vector<std::pair<QObject *, TypedItem *>>::iterator it = activePlugins.begin(); it != activePlugins.end(); it++)
    {   
        if (it->first == plugin)
        {
            activePlugins.erase(it);
            break;
        }
    }
}

void MainWindow::handleOutputPort(QObject *obj)
{
    DataContainer *d = static_cast<DataContainer*>(obj);
    TypedItem *ti = d->getItem();
    PluginHandle ph = d->getPluginHandle();
    QObject *plugin = nullptr;
    
    if (ti->type() == ItemType::CONFIGITEM)
    {
        ItemBase *item = static_cast<ItemBase *>(ti->getData());
        plugin = item->getVizualizer(ph.pluginName);
        
    }
    else if (ti->type() == ItemType::OUTPUTPORT)
    {
        OutputPortItem *outputPort = static_cast<OutputPortItem *>(ti->getData());
        plugin = outputPort->getVizualizer(ph.pluginName);
    }
    
    if (plugin)
    {
        removePlugin(plugin, ti);
        return;
    }
    
    addPlugin(ph, ti);
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
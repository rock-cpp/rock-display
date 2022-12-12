
/* including this first because it uses "emit" and "signals" which are
 * #defined by QT
 *
 * workaround that works in some compilers:
 * #pragma push_macro("emit")
 * #pragma push_macro("signals")
 * #undef emit
 * #undef signals
 */
#include <rtt/transports/corba/TaskContextProxy.hpp>
/*
 * #pragma pop_macro("signals")
 * #pragma pop_macro("emit")
 */

#include "Mainwindow.hpp"

#include <QCursor>
#include <QCloseEvent>
#include <rock_widget_collection/RockWidgetCollection.h>
#include <rtt/base/DataSourceBase.hpp>
#include <rtt/typelib/TypelibMarshallerBase.hpp>
#include <rtt/base/DataSourceBase.hpp>
#include <orocos_cpp/ConfigurationHelper.hpp>

#include "ui_task_inspector_window.h"
#include "Types.hpp"
#include "TypedItem.hpp"
#include "NameServiceItemDelegate.hpp"
#include "ConfigItemHandlerRepository.hpp"
#include "ConfigItemHandler.hpp"
#include "Vizkit3dPluginRepository.hpp"
#include "TaskModel.hpp"
#include "PortItem.hpp"
#include "PropertyItem.hpp"
#include "TaskItem.hpp"
#include "vizplugins/imageviewplugin.hpp"
#include "vizplugins/virtualjoystickplugin.hpp"
#include "configuration.hpp"
#include "ConfigurationSelectDialog.hpp"

void MyVizkit3DWidget::closeEvent(QCloseEvent *ev)
{
    //by default, the closeEvent calls destroy, which releases all windowing
    //resources, including the GL resources needed by osg.
    //instead, use hide() here and abort handling the close event.
    //if this is the last window, quit the application.
    //this also bypasses the WA_DeleteOnClose behaviour that fails due
    //to this widget being a member of MainWindow instead of a heap
    //allocation
    hide();
    bool allHidden = true;
    const QWidgetList topLevelWidgets = QApplication::topLevelWidgets();
    for (QWidget *widget : topLevelWidgets) {
        if (!widget->isHidden())
	{
            allHidden = false;
	    break;
	}
    }
    ev->ignore();
    if(allHidden)
        QApplication::quit();
}

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

    connect(ui->lineEdit, &QLineEdit::textEdited,
            this, &MainWindow::filterTextEdited);

    view->setSortingEnabled(true);
    view->header()->setSortIndicator(0, Qt::AscendingOrder);

    handlerrepo = new ConfigItemHandlerRepository;

    model = new NameServiceModel(handlerrepo, this);
    connect(this, SIGNAL(stopNotifier()), model, SLOT(stop()));
    connect(model, SIGNAL(rowAdded()), this, SLOT(sortTasks()));
    connect(model, &NameServiceModel::itemDataEdited,
            this, QOverload<QModelIndex const &>::of(&MainWindow::itemDataEdited));
    
    TaskModel *initialTasks = new TaskModel(handlerrepo, this);
    model->addTaskModel(initialTasks);

    qRegisterMetaType<std::string>("std::string");
    qRegisterMetaType<RTT::base::DataSourceBase::shared_ptr>("RTT::base::DataSourceBase::shared_ptr");
    view->setModel(model);
    view->header()->resizeSection(0, 250);

    NameServiceItemDelegate *delegate = new NameServiceItemDelegate(this);
    view->setItemDelegate(delegate);

    auto *list = widget3d.getAvailablePlugins();    
    pluginRepo = new Vizkit3dPluginRepository(*list);
    delete list;
    
    additionalPlugins.push_back(new ImageViewPluginHandle);
    additionalPlugins.push_back(new VirtualJoystickPluginHandle);

    std::vector<PluginHandle const *> plugins;
    for (Vizkit3dPluginHandle const *p : pluginRepo->getAllAvailablePlugins())
        plugins.push_back(p);
    plugins.insert(plugins.end(), additionalPlugins.begin(), additionalPlugins.end());
    
    //sort plugins for easy readability 
    std::sort(plugins.begin(), plugins.end(), PluginHandleSortByPluginName(true));
    
    std::vector<std::string> menuWidgets;
    for (const PluginHandle *handle: plugins)
    {
        if (std::find(menuWidgets.begin(), menuWidgets.end(), handle->pluginName) != menuWidgets.end())
        {
            continue;
        }
        
        QSignalMapper* signalMapper = new QSignalMapper (this) ;
        QAction *act = ui->menuWidgets->addAction(QString::fromStdString(handle->pluginName));
        
        menuWidgets.push_back(handle->pluginName);

        connect(act, SIGNAL(triggered()), signalMapper, SLOT(map()));

        signalMapper->setMapping(act, new DataContainer(handle, nullptr));

        connect(signalMapper, SIGNAL(mapped(QObject*)), this, SLOT(openPlugin(QObject*)));
    }
    
    connect(ui->actionAdd_name_service, SIGNAL(triggered()), this, SLOT(addNameService()));
    
    uiUpdateTimer = new QTimer(this);
    connect(uiUpdateTimer, &QTimer::timeout,
            model, [this](){
                model->updateTasks();
            });
    uiUpdateTimer->start(20);

    nameServiceDialog = new AddNameServiceDialog();
    connect(nameServiceDialog, SIGNAL(requestNameServiceAdd(const std::string &)), model, SLOT(addNameService(const std::string &)));
    initialTasks->startNotifier();
    
    view->expand(initialTasks->getRow().first()->index());
    view->expand(initialTasks->getTasks().index());
}

void MainWindow::sortTasks()
{ 
    view->sortByColumn(0, view->header()->sortIndicatorOrder());
}

void MainWindow::filterTextEdited(QString const &text)
{
    //ordinally, i'd reach for QSortFilteredModelProxy, but in this
    //case, the model is not enough of an abstraction and data
    //is passed around bypassing the model interface
    //(meaning the data() function).
    for(int i = 0; i < model->rowCount(QModelIndex()); i++)
    {
        //nameservices
        QModelIndex nsindex = model->index(i,0,QModelIndex());
        QModelIndex tasksindex = model->index(0,0,nsindex);
        for(int j = 0; j < model->rowCount(tasksindex); j++)
        {
            //tasks
            QModelIndex taskindex = model->index(j,0,tasksindex);
            view->setRowHidden(j,tasksindex,
                !model->data(taskindex, Qt::DisplayRole).toString()
                .contains(text, Qt::CaseInsensitive)
            );
        }
    }
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
    PluginHandle const *ph = d->getPluginHandle();
    addPlugin(ph, nullptr);
}

void MainWindow::cleanup()
{   
    emit stopNotifier();
    
    removeAllPlugins();
    widget3d.close();

    delete model;
    delete ui;
    delete pluginRepo;
}

MainWindow::~MainWindow()
{
    removeAllPlugins();
    cleanup();
}

void MainWindow::onExpanded(const QModelIndex& index)
{
    this->setItemExpanded(index, true);
    model->updateTasks(true);
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
        ti->setExpanded(expanded);
    }
}

static const Typelib::Registry* getItemRegistry(QStandardItem *item)
{
    TypedItem *ti = dynamic_cast<TypedItem*>(item);
    if (!ti)
        return nullptr;
    switch (ti->type()) {
        case ItemType::CONFIGITEM:
        case ItemType::OUTPUTPORT:
        case ItemType::INPUTPORT:
        case ItemType::EDITABLEITEM:
        {
            while(item) {
                TypedItem *pti = dynamic_cast<TypedItem*>(item);
                if (!(pti))
                    break;
                if (pti->type() == ItemType::OUTPUTPORT || pti->type() == ItemType::INPUTPORT)
                {
                    RTT::types::TypeInfo const *type = nullptr;
                    if (pti->type() == ItemType::OUTPUTPORT)
                    {
                        auto outputitem = static_cast<OutputPortItem*>(pti->getData());
                        type = outputitem->getPort()->getTypeInfo();
                    }
                    else
                    {
                        auto inputitem = static_cast<InputPortItem*>(pti->getData());
                        type = inputitem->getPort()->getTypeInfo();
                    }
                    auto transport = dynamic_cast<orogen_transports::TypelibMarshallerBase *>(type->getProtocol(orogen_transports::TYPELIB_MARSHALLER_ID));

                    return &transport->getRegistry();
                }
                if (pti->type() != ItemType::CONFIGITEM && pti->type() != ItemType::EDITABLEITEM)
                    break;
                item = item->parent();
            }
            return nullptr;
        }
        default:
            return nullptr;
    }
}

static const Typelib::Type* getItemTypelibType(QModelIndex const &mi, NameServiceModel *model)
{
    QStandardItem *item = model->itemFromIndex(mi);

    TypedItem *ti = dynamic_cast<TypedItem*>(item);
    if (!ti)
        return nullptr;
    if (ti->type() == ItemType::CONFIGITEM)
    {
        return nullptr;
    }
    else if(ti->type() == ItemType::OUTPUTPORT)
    {
        auto outputitem = static_cast<OutputPortItem*>(ti->getData());
        RTT::types::TypeInfo const *type = outputitem->getPort()->getTypeInfo();
        auto transport = dynamic_cast<orogen_transports::TypelibMarshallerBase *>(type->getProtocol(orogen_transports::TYPELIB_MARSHALLER_ID));
        if (!transport)
        {
            return nullptr;
        }
        return transport->getRegistry().get(transport->getMarshallingType());
    }
    else if(ti->type() == ItemType::INPUTPORT)
    {
        auto inputitem = static_cast<InputPortItem*>(ti->getData());
        RTT::types::TypeInfo const *type = inputitem->getPort()->getTypeInfo();
        auto transport = dynamic_cast<orogen_transports::TypelibMarshallerBase *>(type->getProtocol(orogen_transports::TYPELIB_MARSHALLER_ID));
        if (!transport)
        {
            return nullptr;
        }
        return transport->getRegistry().get(transport->getMarshallingType());
    }
    else if (ti->type() == ItemType::EDITABLEITEM)
    {
        auto item = static_cast<ItemBase *>(ti->getData());
        Typelib::Value &val = item->getValueHandle();
        const Typelib::Type &type(val.getType());
        return &type;
    }
    return nullptr;
}

static std::string getItemTypeName(QModelIndex const &mi, NameServiceModel *model)
{
    QStandardItem *item = model->itemFromIndex(mi);

    TypedItem *ti = dynamic_cast<TypedItem*>(item);
    if (!ti)
        return std::string();
    if (ti->type() == ItemType::CONFIGITEM)
    {
        //maybe better store this in the ConfigItem and retrieve from there, instead of from the value column?
        return static_cast<ItemBase *>(ti->getData())->getRow().last()->text().toStdString();
    }
    else if(ti->type() == ItemType::OUTPUTPORT)
    {
        auto outputitem = static_cast<OutputPortItem*>(ti->getData());
        RTT::types::TypeInfo const *type = outputitem->getPort()->getTypeInfo();
        return type->getTypeName();
    }
    else if(ti->type() == ItemType::INPUTPORT)
    {
        auto inputitem = static_cast<InputPortItem*>(ti->getData());
        RTT::types::TypeInfo const *type = inputitem->getPort()->getTypeInfo();
        return type->getTypeName();
    }
    else if (ti->type() == ItemType::EDITABLEITEM)
    {
        auto item = static_cast<ItemBase *>(ti->getData());
        Typelib::Value &val = item->getValueHandle();
        const Typelib::Type &type(val.getType());
        return type.getName();
    }
    return std::string();
}

void MainWindow::prepareMenu(const QPoint & pos)
{
    QModelIndex mi = view->indexAt(pos);
    QStandardItem *item = model->itemFromIndex(mi);
    QMenu *menu = new QMenu(this);
    //there is also addSection, but the styling is lacking.
    QLabel* label = new QLabel(tr(""), menu);
    label->setAlignment(Qt::AlignCenter);
    QWidgetAction* labelAction = new QWidgetAction(menu);
    labelAction->setDefaultWidget(label);
    menu->setMinimumWidth(100);
    menu->setMinimumHeight(35);

    if (TypedItem *ti = dynamic_cast<TypedItem*>(item))
    {
        switch (ti->type()) {
            case ItemType::TASK:
            {
                TaskItem *titem = static_cast<TaskItem*>(ti->getData());

                task = titem->getTaskContext();

                menu->addAction(labelAction);
                label->setText(tr("<b>Actions</b>"));
                QAction *act = menu->addAction("Activate");
                QAction *sta = menu->addAction("Start");
                QAction *sto = menu->addAction("Stop");
                QAction *con = menu->addAction("Configure");
                QAction *load = menu->addAction(tr("Load Configuration from file..."));
                QAction *save = menu->addAction(tr("Save Configuration to file..."));

                connect(act, SIGNAL(triggered()), this, SLOT(activateTask()));
                connect(sta, SIGNAL(triggered()), this, SLOT(startTask()));
                connect(sto, SIGNAL(triggered()), this, SLOT(stopTask()));
                connect(con, SIGNAL(triggered()), this, SLOT(configureTask()));
                connect(load, &QAction::triggered,
                    this, [this,titem](){
                        QString filename = QFileDialog::getOpenFileName(this, tr("Load Configuration..."), QString(), "Configuration file (*.yaml *.yml)");
                        if (filename.isEmpty())
                        {
                            return;
                        }
                        orocos_cpp::ConfigurationHelper helper;
                        libConfig::MultiSectionConfiguration mcfg;
                        mcfg.loadNoBundle(filename.toStdString(), task->getName());
                        ConfigurationSelectDialog *csd = new ConfigurationSelectDialog(this);
                        std::vector<std::string> sections;
                        for(auto &c : mcfg.getSubsections())
                        {
                            sections.push_back(c.first);
                        }
                        if (sections.size() > 1)
                        {
                            csd->setSelectionOptions(sections);
                            if(csd->exec() == QDialog::Rejected)
                            {
                                return;
                            }
                            sections = csd->getSelectionOptions();
                        }

                        libConfig::Configuration config = mcfg.getConfig(sections);
                        csd->deleteLater();
                        try
                        {
                            if (!helper.applyConfig(task, config))
                            {
                                QMessageBox::critical(this,tr("Load Configuration"),tr("Loading configuration failed"));
                            }
                            else
                            {
                                titem->updateProperties();
                            }
                        }
                        catch (std::runtime_error const &e)
                        {
                            QMessageBox::critical(this,tr("Load Configuration"),tr("Loading configuration failed:\n%1").arg(e.what()));
                        }

                    });
                connect(save, &QAction::triggered,
                    this, [this,titem](){
                        QString filename = QFileDialog::getSaveFileName(this, tr("Save Configuration..."), QString(), "Configuration file (*.yaml *.yml)");
                        if (filename.isEmpty())
                        {
                            return;
                        }

                        if (!save_configuration(task, titem, filename.toStdString()))
                        {
                            QMessageBox::critical(this,tr("Save Configuration"),tr("Saving configuration failed"));
                        }
                    });

            }
                break;
                
            case ItemType::CONFIGITEM:
            case ItemType::OUTPUTPORT:
            case ItemType::INPUTPORT:
            case ItemType::EDITABLEITEM:
            {
                std::string typeName = getItemTypeName(mi, model);
                const Typelib::Type* typelibType = getItemTypelibType(mi, model);
                const Typelib::Registry* registry = getItemRegistry(model->itemFromIndex(mi));
                VisualizerAdapter *viz = static_cast<ItemBase*>(ti->getData());

                ItemBase* itembase = nullptr;
                if(ti->type() == ItemType::CONFIGITEM || ti->type() == ItemType::EDITABLEITEM)
                    itembase = static_cast<ItemBase*>(ti->getData());
                else if (ti->type() == ItemType::INPUTPORT || ti->type() == ItemType::OUTPUTPORT)
                {
                    itembase = static_cast<PortItem *>(ti->getData())->getItemBase().get();
                }

                if(itembase)
                {
                    for(auto &h : itembase->getHandlerStack())
                    {
                        if (h->flags() & ConfigItemHandler::Flags::ContextMenuItems) {
                            h->addContextMenuEntries(menu, mi);
                        }
                    }
                }
                
                if (ItemBase::marshalled2Typelib.find(typeName) != ItemBase::marshalled2Typelib.end())
                {
                    typeName = ItemBase::marshalled2Typelib[typeName];
                }
                
                std::vector<PluginHandle const *> handles;
                for (auto &p : pluginRepo->getPluginsForType(typeName, registry))
                {
                    handles.push_back(p);
                }
                if (typelibType)
                {
                    for (PluginHandle *additionalPlugin: additionalPlugins)
                    {
                        if (additionalPlugin->probe(*typelibType, registry))
                        {
                            handles.push_back(additionalPlugin);
                        }
                    }
                }

                menu->addAction(labelAction);
                label->setText(tr("<b>Plugins</b>"));

                for (const PluginHandle *handle: handles)
                {
                    QSignalMapper* signalMapper = new QSignalMapper(this);
                    QAction *act = nullptr;
                    if (viz->hasVisualizer(handle->pluginName))
                    {
                        act = menu->addAction(tr("remove %1").arg(QString::fromStdString(handle->pluginName)));
                    }
                    else
                    {
                        act = menu->addAction(QString::fromStdString(handle->pluginName));
                    }

                    connect(act, SIGNAL(triggered()), signalMapper, SLOT(map()));
                    signalMapper->setMapping(act, new DataContainer(handle, ti));

                    connect(signalMapper, SIGNAL(mapped(QObject*)), this, SLOT(handlePort(QObject*)));
                }
                
                if (handles.size() == 0)
                {
                    QWidgetAction* a = new QWidgetAction(menu);
                    a->setDefaultWidget(new QLabel(tr("none"), this));
                    menu->addAction(a);
                }
            }
                break;
            default:
                return;
        }
      
        menu->exec(QCursor::pos());
    }
}

void MainWindow::addPlugin(PluginHandle const *ph, TypedItem* ti)
{   
    VizHandle *nh = ph->createViz();

    if (!nh)
    {
        return;
    }
    
    QWidget *widget = nh->getStandaloneWidget();
    if (widget)
    {
        widget->show();
    }
    QObject *v3dplugin = nh->getVizkit3dPluginObject();
    if (v3dplugin)
    {
        widget3d.addPlugin(v3dplugin);
        widget3d.show();
    }

    activePlugins.push_back(std::make_pair(nh, ti));
    
    if (!ti)
    {
        return;
    }

    VisualizerAdapter *viz = nullptr;
    Typelib::Value value;
    
    if (ti->type() == ItemType::OUTPUTPORT || ti->type() == ItemType::INPUTPORT)
    {
        PortItem *pi = static_cast<PortItem *>(ti->getData());
        viz = pi;
        value = pi->getValueHandle();
    }
    else if (ti->type() == ItemType::CONFIGITEM || ti->type() == ItemType::EDITABLEITEM)
    {
        ItemBase * item = static_cast<ItemBase *>(ti->getData());
        viz = item;
        value = item->getValueHandle();
    }
        
    if (viz)
    {
        viz->addPlugin(ph->pluginName, nh);
        connect(nh,&VizHandle::editableChanged,
                this, [this,ti](void *data, bool force_send) {
                    (void)data;
                    this->itemDataEdited(ti, force_send);
                });
        connect(nh, &VizHandle::closing,
                this, [this,ti](VizHandle *nh) {
                    removePlugin(nh,ti);
                });
        if (value.getData())
        {
            nh->updateVisualizer(value.getData());
            nh->updateEditable(value.getData());
        }
    }
}

void MainWindow::removePlugin(VizHandle *plugin, TypedItem *ti)
{
    QObject *v3dplugin = plugin->getVizkit3dPluginObject();
    if (v3dplugin)
    {
        widget3d.removePlugin(v3dplugin);
    }
    QWidget *widget = plugin->getStandaloneWidget();
    if (widget)
    {
        widget->close();
    }
    
    if (ti)
    {
        if (ti->type() == ItemType::CONFIGITEM || ti->type() == ItemType::EDITABLEITEM)
        {
            ItemBase *item = static_cast<ItemBase *>(ti->getData());
            item->removeVisualizer(plugin);
        }
        else if (ti->type() == ItemType::OUTPUTPORT || ti->type() == ItemType::INPUTPORT)
        {
            PortItem *portitem = static_cast<PortItem *>(ti->getData());
            portitem->removeVisualizer(plugin);
        }
    }
    
    for (std::vector<std::pair<VizHandle *, TypedItem *>>::iterator it = activePlugins.begin(); it != activePlugins.end(); it++)
    {   
        if (it->first == plugin)
        {
            activePlugins.erase(it);
            delete plugin;
            break;
        }
    }
}

void MainWindow::handlePort(QObject *obj)
{
    DataContainer *d = static_cast<DataContainer*>(obj);
    TypedItem *ti = d->getItem();
    PluginHandle const *ph = d->getPluginHandle();
    
    VisualizerAdapter *viz = nullptr;
        
    if (ti->type() == ItemType::OUTPUTPORT || ti->type() == ItemType::INPUTPORT)
    {
        viz = static_cast<PortItem *>(ti->getData());
    }
    else if (ti->type() == ItemType::CONFIGITEM || ti->type() == ItemType::EDITABLEITEM)
    {
        viz = static_cast<ItemBase *>(ti->getData());
    }
            
    if (viz)
    {
        VizHandle *plugin = viz->getVisualizer(ph->pluginName);
        if (plugin)
        {
            removePlugin(plugin, ti);
            return;
        }
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

void MainWindow::itemDataEdited(const QModelIndex &index)
{
    itemDataEdited(model->itemFromIndex(index));
}

void MainWindow::itemDataEdited(QStandardItem *qitem, bool forceSend)
{
    //This very probably is an EDITABLEITEM, so scan for the INPUTPORT (or PROPERTYITEM)
    //if it is not, or we cannot find INPUTPORT (or PROPERTYITEM), we just bail out.
    InputPortItem *inputitem = nullptr;
    PropertyItem *propertyitem = nullptr;
    while(qitem) {
        TypedItem *pti = dynamic_cast<TypedItem*>(qitem);
        if (!(pti))
            return;
        if (pti->type() == ItemType::INPUTPORT)
        {
            inputitem = static_cast<InputPortItem*>(pti->getData());
            break;
        }
        else if (pti->type() == ItemType::PROPERTYITEM)
        {
            propertyitem = static_cast<PropertyItem*>(pti->getData());
            break;
        }
        else if (pti->type() == -1)
        {
            // continue the scan for the parent. next one would be the task.
        }
        else if (pti->type() != ItemType::EDITABLEITEM)
        {
            return;
        }
        qitem = qitem->parent();
    }

    if(inputitem)
    {
        inputitem->updataValue(true);
        if (forceSend)
        {
            //send it now, the rest of the logic takes care of removing
            //the confirm button box (if any) and coloring.
            inputitem->sendCurrentData();
        }
        if(!inputitem->compareAndMarkData())
        {
            if (changeconfirms.find(inputitem) == changeconfirms.end())
            {
                QDialogButtonBox *box = new QDialogButtonBox(this);
                box->addButton(tr("Accept"), QDialogButtonBox::AcceptRole);
                box->addButton(tr("Reject"), QDialogButtonBox::RejectRole);
                box->setAutoFillBackground(true);

                changeconfirms.insert(std::make_pair(inputitem, static_cast<QWidget*>(box)));
                view->setIndexWidget(inputitem->getValueItem()->index(), box);
                connect(box, &QDialogButtonBox::accepted,
                        this, [inputitem, this, box]
                {
                    inputitem->sendCurrentData();
                    inputitem->compareAndMarkData();
                    changeconfirms.erase(changeconfirms.find(inputitem));
                    view->setIndexWidget(inputitem->getValueItem()->index(), nullptr);
                    box->deleteLater();
                });
                connect(box, &QDialogButtonBox::rejected,
                        this, [inputitem, this, box]
                {
                    inputitem->restoreOldData();
                    inputitem->compareAndMarkData();
                    changeconfirms.erase(changeconfirms.find(inputitem));
                    view->setIndexWidget(inputitem->getValueItem()->index(), nullptr);
                    box->deleteLater();
                });
            }
        }
        else
        {
            auto it = changeconfirms.find(inputitem);
            if (it != changeconfirms.end()) {
                view->setIndexWidget(inputitem->getValueItem()->index(), nullptr);
                delete it->second;
                changeconfirms.erase(it);
            }
        }
    }
    if (propertyitem)
    {
        propertyitem->setCurrentData();
    }
}


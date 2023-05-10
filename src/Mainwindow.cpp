
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
#include "TaskModel.hpp"
#include "PortItem.hpp"
#include "PropertyItem.hpp"
#include "TaskItem.hpp"
#include "configuration.hpp"
#include "ConfigurationSelectDialog.hpp"
#include "vizplugins/artificialhorizonplugin.hpp"
#include "vizplugins/imageviewplugin.hpp"
#include "vizplugins/plot2dplugin.hpp"
#include "vizplugins/orientationviewplugin.hpp"
#include "vizplugins/rangeviewplugin.hpp"
#include "vizplugins/sonardisplayplugin.hpp"
#include "vizplugins/sonarviewplugin.hpp"
#include "vizplugins/sonarwidgetplugin.hpp"
#include "vizplugins/streamalignerwidgetplugin.hpp"
#include "vizplugins/virtualjoystickplugin.hpp"
#include "vizplugins/vizkitplugin_p.hpp"
#include "vizplugins/vizkit3dplugins.hpp"
#include <unordered_set>
#include <QTimer>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QLabel>
#include <QWidgetAction>
#include <QMessageBox>
#include <QFileDialog>

void PluginWidgetQMainWindow::closeEvent(QCloseEvent *ev)
{
    //by default, the closeEvent calls destroy, which releases all windowing
    //resources, including the GL resources needed by osg.
    //instead, use hide() here and abort handling the close event.
    //notify the MainWindow of this for potential reuse if this window.
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

    emit closing();
}

RTTTypelibTypeConverter::RTTTypelibTypeConverter(orogen_transports::TypelibMarshallerBase *transport,
        std::string const &nativeTypename)
    : nativeTypename(nativeTypename), transport(transport)
{
    transportHandle = transport->createSample();
}

RTTTypelibTypeConverter::~RTTTypelibTypeConverter() {
    orogen_transports::TypelibMarshallerBase::Handle *transportHandle =
        static_cast<orogen_transports::TypelibMarshallerBase::Handle *>(this->transportHandle);

    transport->deleteHandle(transportHandle);
}

TypeConverter::ConversionResult RTTTypelibTypeConverter::convertToResult(Typelib::Value const &value,
        const Typelib::Registry *registry)
{
    TypeConverter::ConversionResult result;
    orogen_transports::TypelibMarshallerBase::Handle *transportHandle =
        static_cast<orogen_transports::TypelibMarshallerBase::Handle *>(this->transportHandle);

    transport->setTypelibSample(transportHandle, value);

    const Typelib::Type *type3 = transport->getRegistry().get(nativeTypename);
    result.convertedRawPtr = transport->getOrocosSample(transportHandle);

    result.convertedValue = Typelib::Value(result.convertedRawPtr, *type3);

    return result;
}

void RTTTypelibTypeConverter::refreshFromResult(Typelib::Value &orig_value)
{
    orogen_transports::TypelibMarshallerBase::Handle *transportHandle =
        static_cast<orogen_transports::TypelibMarshallerBase::Handle *>(this->transportHandle);

    transport->refreshTypelibSample(transportHandle);

    Typelib::Value new_val(transport->getTypelibSample(transportHandle), orig_value.getType());
    Typelib::copy(orig_value, new_val);
}

TypeConverter::ConversionResult RTTAliasTypeConverter::convertToResult(Typelib::Value const &value,
        const Typelib::Registry *registry)
{
    TypeConverter::ConversionResult result;

    result.convertedRawPtr = value.getData();
    if (registry)
    {
        const Typelib::Type *type3 = registry->get(aliasTypename);

        result.convertedValue = Typelib::Value(result.convertedRawPtr, *type3);
    }
    else
    {
        result.convertedValue = Typelib::Value();
    }

    return result;
}

void RTTAliasTypeConverter::refreshFromResult(Typelib::Value &orig_value)
{
}

std::unique_ptr<TypeConverter> RTTTypelibTypeConverterFactory::createConverter() const
{
    return std::unique_ptr<RTTTypelibTypeConverter>(new RTTTypelibTypeConverter(transport, nativeTypename));
}

std::unique_ptr<TypeConverter> RTTAliasTypeConverterFactory::createConverter() const
{
    return std::unique_ptr<RTTAliasTypeConverter>(new RTTAliasTypeConverter(aliasTypename));
}

FieldVizHandle::~FieldVizHandle()
{
}

void FieldVizHandle::updateVisualizer(Typelib::Value const &value)
{
    if (outputportfield)
    {
        if (!valueHandle)
        {
            valueHandle = new rockdisplay::ValueHandleImpl
            (value, fieldHandle, this);
            if (converterfactory)
            {
                converter = converterfactory->createConverter();
            }
        }
        fieldHandle->setType(&value.getType());
        if (!converter)
        {
            valueHandle->setValue(value, value.getData());
        }
        else
        {
            const Typelib::Registry *registry = fieldHandle->getRegistry();

            TypeConverter::ConversionResult res;
            try
            {
                res = converter->convertToResult(value, registry);
            }
            catch (std::exception const &e)
            {
                return;
            }

            valueHandle->setValue(res.convertedValue, res.convertedRawPtr);
        }
        outputportfield->updateOutputPort(valueHandle);
    }
    if (inputportfield)
    {
        updateEditable(value);
    }
    if (propertyfield)
    {
        updateEditable(value);
    }
}

void FieldVizHandle::updateEditable(Typelib::Value const &value)
{
    orig_value = value;
    if (!valueHandle)
    {
        valueHandle = new rockdisplay::ValueHandleImpl
        (value, fieldHandle, this);
        if (converterfactory)
        {
            converter = converterfactory->createConverter();
        }
        connect(valueHandle, &rockdisplay::ValueHandleImpl::editedSignal,
                this, &FieldVizHandle::edited);
    }
    fieldHandle->setType(&value.getType());
    if (!converter)
    {
        if (inputportfield)
        {
            valueHandle->setValue(value, value.getData());
        }
        if (propertyfield)
        {
            valueHandle->setValue(value, value.getData());
        }
    }
    else
    {
        const Typelib::Registry *registry = fieldHandle->getRegistry();

        TypeConverter::ConversionResult res;
        try
        {
            res = converter->convertToResult(value, registry);
        }
        catch (std::exception const &e)
        {
            return;
        }
        valueHandle->setValue(res.convertedValue, res.convertedRawPtr);
    }
    if (inputportfield)
    {
        inputportfield->updateInputPort(valueHandle);
    }
    if (propertyfield)
    {
        propertyfield->updateProperty(valueHandle);
    }
}

void FieldVizHandle::edited(bool force_send)
{
    if (converter)
    {
        converter->refreshFromResult(orig_value);
    }
    emit editableChanged(valueHandle->getValue(), force_send);
}

MainWindow::MainWindow(orocos_cpp::OrocosCpp &orocos, QWidget *parent) :
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

    view->sortByColumn(0,Qt::AscendingOrder);
    view->setSortingEnabled(true);

    handlerrepo = new ConfigItemHandlerRepository;

    model = new NameServiceModel(handlerrepo, orocos, this);
    connect(this, SIGNAL(stopNotifier()), model, SLOT(stop()));
    connect(model, SIGNAL(rowAdded()), this, SLOT(sortTasks()));
    connect(model, &NameServiceModel::dataChanged,
            this, [this](const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector< int >& roles)
    {
        (void)bottomRight;
        (void)roles;
        //catch name changes in tasks. this used to be an issue because the TaskItem constructor defered
        //setting the name until the first update, while rowAdded gets fired earlier.
        if (topLeft.parent().parent().isValid() && !topLeft.parent().parent().parent().isValid())
        {
            sortTasks();
        }
    });
    connect(model, &NameServiceModel::itemDataEdited,
            this, QOverload<QModelIndex const &>::of(&MainWindow::itemDataEdited));
    
    TaskModel *initialTasks = new TaskModel(handlerrepo, orocos, this);
    model->addTaskModel(initialTasks);

    qRegisterMetaType<std::string>("std::string");
    qRegisterMetaType<RTT::base::DataSourceBase::shared_ptr>("RTT::base::DataSourceBase::shared_ptr");
    qRegisterMetaType<Typelib::Value>("Typelib::Value");
    view->setModel(model);
    view->header()->resizeSection(0, 300);

    NameServiceItemDelegate *delegate = new NameServiceItemDelegate(this);
    view->setItemDelegate(delegate);

    plugins.push_back(new rock_display::ArtificialHorizonPlugin);
    plugins.push_back(new rock_display::ImageViewPlugin);
    plugins.push_back(new rock_display::Plot2dPlugin);
    plugins.push_back(new rock_display::OrientationViewPlugin);
    plugins.push_back(new rock_display::RangeViewPlugin);
    plugins.push_back(new rock_display::SonarDisplayPlugin);
    plugins.push_back(new rock_display::SonarViewPlugin);
    plugins.push_back(new rock_display::SonarWidgetPlugin);
    plugins.push_back(new rock_display::StreamAlignerWidgetPlugin);
    plugins.push_back(new rock_display::VirtualJoystickPlugin);
    plugins.push_back(new rock_display::Vizkit3DPlugins);

    std::sort(plugins.begin(), plugins.end(), [](rockdisplay::vizkitplugin::Plugin*a, rockdisplay::vizkitplugin::Plugin*b){
        std::string const &_a = a->getName();
        std::string const &_b = b->getName();
        return std::lexicographical_compare(_a.begin(), _a.end(),
                                            _b.begin(), _b.end());
    });

    regenerateWidgetsMenu();

    connect(ui->actionAdd_name_service, SIGNAL(triggered()), this, SLOT(addNameService()));
    
    uiUpdateTimer = new QTimer(this);
    connect(uiUpdateTimer, &QTimer::timeout,
            model, [this](){
                model->updateTasks(false);
            });
    uiUpdateTimer->start(20);

    nameServiceDialog = new AddNameServiceDialog();
    connect(nameServiceDialog, SIGNAL(requestNameServiceAdd(const std::string &)), model, SLOT(addNameService(const std::string &)));
    initialTasks->startNotifier();
    
    view->expand(initialTasks->getRow().first()->index());
    view->expand(initialTasks->getTasks().index());



    auto tir = RTT::types::TypeInfoRepository::Instance();
    for(auto nativeType : tir->getTypes()) {
        auto ti = tir->type(nativeType);
        auto transport = dynamic_cast<orogen_transports::TypelibMarshallerBase *>(ti->getProtocol(orogen_transports::TYPELIB_MARSHALLER_ID));
        if (! transport)
        {
            continue;
        }
        auto portType = transport->getMarshallingType();
        if (portType != nativeType)
        {
            typeconverters.insert(std::make_pair(portType,
                                                 new RTTTypelibTypeConverterFactory(transport,
                                                         nativeType)));
        }

        for (auto &t : ti->getTypeNames())
        {
            if (t != nativeType)
            {
                typeconverters.insert(std::make_pair(nativeType,
                                                     new RTTAliasTypeConverterFactory(t)));
            }
        }
    }

    for(auto &tc : typeconverters) {
        printf("Know how to convert from %s to %s\n",
               tc.first.c_str(), tc.second->getResultTypename().c_str());
    }

}

void MainWindow::regenerateWidgetsMenu()
{
    ui->menuWidgets->clear();

    std::unordered_set<std::string> menuWidgets;

    for(auto &handle : plugins) {
        if (menuWidgets.find(handle->getName()) != menuWidgets.end())
        {
            continue;
        }

        //find all widgets associated with this plugin
        std::vector<rockdisplay::vizkitplugin::Widget*> widgets;
        for(auto &widget : this->widgets) {
            if(widget.plugin != handle)
                continue;
            widgets.push_back(widget.widget);
        }

        std::sort(widgets.begin(), widgets.end(), [this](rockdisplay::vizkitplugin::Widget * a, rockdisplay::vizkitplugin::Widget * b)
        {
            std::string const &_a = widgetWindows[a]->windowTitle().toStdString();
            std::string const &_b = widgetWindows[b]->windowTitle().toStdString();
            return std::lexicographical_compare(_a.begin(), _a.end(),
                                                _b.begin(), _b.end());
        });

        menuWidgets.insert(handle->getName());

        bool offerCreate = widgets.size() == 0 ||
                (handle->getFlags() & rockdisplay::vizkitplugin::Plugin::PreferSingleWidget) == 0;
        if (offerCreate)
        {
            QAction *act = ui->menuWidgets->addAction(QString::fromStdString(handle->getName()));
            connect(act, &QAction::triggered,
                    this, [this, handle]()
            {
                createWidget(handle);
            });
        }

        bool haveSubmenu = offerCreate && widgets.size() != 0;

        std::vector<std::string> subplugins = handle->getStandaloneSubplugins();

        std::sort(subplugins.begin(), subplugins.end(), [](std::string const &a, std::string const &b)
        {
            return std::lexicographical_compare(a.begin(), a.end(),
                                                b.begin(), b.end());
        });

        subplugins.erase(std::unique(subplugins.begin(), subplugins.end()), subplugins.end());

        for(auto &name : subplugins) {
            QString menu_title = QString::fromStdString(handle->getName());

            if (!name.empty())
            {
                menu_title = tr("%1: %2", "Plugin: Subplugin").
                             arg(QString::fromStdString(handle->getName())).
                             arg(QString::fromStdString(name));
            }

            QMenu *receiver_menu = ui->menuWidgets;
            if (haveSubmenu)
            {
                receiver_menu = ui->menuWidgets->addMenu(menu_title);
            }
            if (offerCreate)
            {
                QAction *a;
                if (haveSubmenu)
                {
                    a = receiver_menu->addAction(tr("Create new widget"));
                }
                else
                {
                    a = receiver_menu->addAction(menu_title);
                }
                connect(a, &QAction::triggered,
                        this, [this, handle, name]()
                {
                    rockdisplay::vizkitplugin::Widget *widget =
                        createWidget(handle);
                    if (widget)
                    {
                        addFieldToWidget(handle, name, widget, nullptr, nullptr);
                    }
                });
            }


            for (auto &w : widgets)
            {
                QAction *a;
                if (haveSubmenu)
                {
                    a = receiver_menu->addAction(tr("Add to %1").arg(widgetWindows[w]->windowTitle()));
                }
                else
                {
                    a = receiver_menu->addAction(menu_title);
                }
                rockdisplay::vizkitplugin::Widget *widget = w;
                connect(a, &QAction::triggered,
                        this, [this, handle, widget, name]()
                {
                    addFieldToWidget(handle, name, widget, nullptr, nullptr);
                });
            }
        }
    }
}

void MainWindow::sortTasks()
{
    view->setSortingEnabled(false);
    view->setSortingEnabled(true);
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
    while (!widgets.empty())
    {
        destroyWidget(widgets.front().plugin, widgets.front().widget);
    }
}

void MainWindow::addNameService()
{
    nameServiceDialog->show();
    nameServiceDialog->exec();
}

void MainWindow::cleanup()
{   
    emit stopNotifier();
    
    removeAllPlugins();

    delete model;
    delete ui;
}

MainWindow::~MainWindow()
{
    removeAllPlugins();
    cleanup();
    for (auto &p : plugins)
    {
        delete p;
    }
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
    //scan for a port/property item, tell it to update its value display using old data
    if(expanded) {
        while (item)
        {
            TypedItem *ti = dynamic_cast<TypedItem *>(item);
            if (!ti)
            {
                item = item->parent();
                continue;
            }
            switch (ti->type())
            {
                case QStandardItem::ItemType::Type:
                case ItemType::CONFIGITEM:
                case ItemType::EDITABLEITEM:
                {
                    item = item->parent();
                    break;
                }
                case ItemType::OUTPUTPORT:
                {
                    auto outputitem = static_cast<OutputPortItem *>(ti->getData());
                    outputitem->updataValue(true);
                    item = nullptr;
                    break;
                }
                case ItemType::INPUTPORT:
                {
                    auto inputitem = static_cast<InputPortItem *>(ti->getData());
                    inputitem->updataValue(true);
                    item = nullptr;
                    break;
                }
                case ItemType::PROPERTYITEM:
                {
                    auto propertyitem = static_cast<PropertyItem *>(ti->getData());
                    propertyitem->updataValue();
                    item = nullptr;
                    break;
                }
                case ItemType::TASK:
                case ItemType::NAMESERVICE:
                default:
                    item = nullptr;
                    break;
            }
        }
    }
}

struct ItemInfo
{
    orocos_cpp::NameService *nameService;
    RTT::corba::TaskContextProxy *taskContextProxy;
    Typelib::Registry const *registry;
    RTT::base::OutputPortInterface* outputPort;
    RTT::base::InputPortInterface* inputPort;
    RTT::base::PropertyBase *property;
    Typelib::Type const *typelibType;
    std::string fieldName;
};

static ItemInfo getItemInfo(QStandardItem *item)
{
    QStandardItem *queried_item = item;
    ItemInfo info;
    info.nameService = nullptr;
    info.taskContextProxy = nullptr;
    info.registry = nullptr;
    info.outputPort = nullptr;
    info.inputPort = nullptr;
    info.property = nullptr;
    info.typelibType = nullptr;
    info.fieldName = "";
    while (item)
    {
        TypedItem *ti = dynamic_cast<TypedItem *>(item);
        if (!ti)
        {
            item = item->parent();
            continue;
        }
        switch (ti->type())
        {
            case QStandardItem::ItemType::Type:
            {
                item = item->parent();
                break;
            }
            case ItemType::CONFIGITEM:
            case ItemType::EDITABLEITEM:
            {
                auto ib = static_cast<ItemBase *>(ti->getData());
                if (item == queried_item)
                {
                    Typelib::Value &val = ib->getValueHandle();
                    const Typelib::Type &type(val.getType());
                    info.typelibType = &type;
                }
                if(!info.fieldName.empty())
                {
                    info.fieldName = ib->getName()->text().toStdString()+"."+info.fieldName;
                }
                else
                {
                    info.fieldName = ib->getName()->text().toStdString();
                }
                item = item->parent();
                break;
            }
            case ItemType::OUTPUTPORT:
            {
                auto outputitem = static_cast<OutputPortItem *>(ti->getData());
                info.outputPort = dynamic_cast<RTT::base::OutputPortInterface *>(outputitem->getPort());
                RTT::types::TypeInfo const *type = outputitem->getPort()->getTypeInfo();
                auto transport = dynamic_cast<orogen_transports::TypelibMarshallerBase *>(type->getProtocol(orogen_transports::TYPELIB_MARSHALLER_ID));

                if (transport)
                {
                    info.registry = &transport->getRegistry();
                    if (item == queried_item)
                    {
                        info.typelibType = transport->getRegistry().get(transport->getMarshallingType());
                    }
                }

                item = item->parent();
                break;
            }
            case ItemType::INPUTPORT:
            {
                auto inputitem = static_cast<InputPortItem *>(ti->getData());
                info.inputPort = dynamic_cast<RTT::base::InputPortInterface *>(inputitem->getPort());
                RTT::types::TypeInfo const *type = inputitem->getPort()->getTypeInfo();
                auto transport = dynamic_cast<orogen_transports::TypelibMarshallerBase *>(type->getProtocol(orogen_transports::TYPELIB_MARSHALLER_ID));

                if (transport)
                {
                    info.registry = &transport->getRegistry();
                    if (item == queried_item)
                    {
                        info.typelibType = transport->getRegistry().get(transport->getMarshallingType());
                    }
                }

                item = item->parent();
                break;
            }
            case ItemType::PROPERTYITEM:
            {
                auto propertyitem = static_cast<PropertyItem *>(ti->getData());
                info.property = propertyitem->getProperty();
                RTT::types::TypeInfo const *type = propertyitem->getProperty()->getTypeInfo();
                auto transport = dynamic_cast<orogen_transports::TypelibMarshallerBase *>(type->getProtocol(orogen_transports::TYPELIB_MARSHALLER_ID));

                if (transport)
                {
                    info.registry = &transport->getRegistry();
                    if (item == queried_item)
                    {
                        info.typelibType = transport->getRegistry().get(transport->getMarshallingType());
                    }
                }

                item = item->parent();
                break;
            }
            case ItemType::TASK:
            {
                TaskItem *titem = static_cast<TaskItem *>(ti->getData());

                info.taskContextProxy = titem->getTaskContext();
                item = item->parent();
                break;
            }
            case ItemType::NAMESERVICE:
            {
                TaskModel *titem = static_cast<TaskModel *>(ti->getData());

                info.nameService = titem->getNameService();
                item = nullptr;
                break;
            }
            default:
                assert(false);
                break;
        }
    }
    assert(info.nameService);
    return info;
}

namespace std {
  template <> struct hash<std::pair<rockdisplay::vizkitplugin::Widget *, std::string>>
  {
    size_t operator()(const std::pair<rockdisplay::vizkitplugin::Widget *, std::string> & x) const
    {
      return hash<rockdisplay::vizkitplugin::Widget *>()(x.first) ^
      hash<std::string>()(x.second);
    }
  };
}

struct Conversion
{
    std::string typeName;
    const Typelib::Type *typelibType;
    TypeConverterFactory *converterfactory;
    std::unique_ptr<rockdisplay::FieldDescriptionImpl> outputportFieldDesc;
    std::unique_ptr<rockdisplay::FieldDescriptionImpl> inputportFieldDesc;
    std::unique_ptr<rockdisplay::FieldDescriptionImpl> propertyFieldDesc;
    explicit Conversion(std::string typeName, const Typelib::Type *typelibType,
                        TypeConverterFactory *converterfactory,
                        RTT::base::OutputPortInterface *outputport,
                        RTT::base::InputPortInterface *inputport,
                        RTT::base::PropertyBase *property,
                        RTT::corba::TaskContextProxy *taskContextProxy,
                        orocos_cpp::NameService *nameService,
                        const Typelib::Registry *registry,
                        std::string const &fieldName, TypedItem *ti)
        : typeName(typeName), typelibType(typelibType),
          converterfactory(converterfactory),
          outputportFieldDesc(outputport ?
                              new rockdisplay::FieldDescriptionImpl(outputport,
                                      taskContextProxy, nameService,
                                      typelibType, registry, fieldName, ti) :
                              nullptr),
          inputportFieldDesc(inputport ?
                             new rockdisplay::FieldDescriptionImpl(inputport,
                                     taskContextProxy, nameService,
                                     typelibType, registry, fieldName, ti) :
                             nullptr),
          propertyFieldDesc(property ?
                            new rockdisplay::FieldDescriptionImpl(property,
                                    taskContextProxy, nameService,
                                    typelibType, registry, fieldName, ti) :
                            nullptr)
    {
    }
    Conversion(Conversion const &) = delete;
    Conversion(Conversion && oth)
        : typeName(oth.typeName), typelibType(oth.typelibType),
          converterfactory(oth.converterfactory),
          outputportFieldDesc(std::move(oth.outputportFieldDesc)),
          inputportFieldDesc(std::move(oth.inputportFieldDesc)),
          propertyFieldDesc(std::move(oth.propertyFieldDesc))
    {}
    Conversion &operator=(Conversion const &) = delete;
    Conversion &operator=(Conversion && oth) {
        typeName = oth.typeName;
        typelibType = oth.typelibType;
        converterfactory = oth.converterfactory;
        outputportFieldDesc = std::move(oth.outputportFieldDesc);
        inputportFieldDesc = std::move(oth.inputportFieldDesc);
        propertyFieldDesc = std::move(oth.propertyFieldDesc);
        return *this;
    }
};

static std::vector<Conversion> createTypeCandidates(
    std::unordered_multimap< std::string, TypeConverterFactory * > const
    &typeconverters,
    TypedItem *ti)
{

    ItemInfo iteminfo = getItemInfo(ti);
    const Typelib::Registry *registry = iteminfo.registry;
    RTT::corba::TaskContextProxy *taskContextProxy = iteminfo.taskContextProxy;
    orocos_cpp::NameService *nameService = iteminfo.nameService;
    assert(nameService);
    std::string fieldName = iteminfo.fieldName;
    const Typelib::Type *typelibType = iteminfo.typelibType;
    auto outputport = iteminfo.outputPort;
    auto inputport = iteminfo.inputPort;
    auto property = iteminfo.property;

    std::string typeName;
    if (typelibType)
    {
        typeName = typelibType->getName();
    }
    else if (outputport)
    {
        typeName = outputport->getTypeInfo()->getTypeName();
    }
    else if (inputport)
    {
        typeName = inputport->getTypeInfo()->getTypeName();
    }
    else if (property)
    {
        typeName = property->getTypeInfo()->getTypeName();
    }
    else
    {
        assert(false && "TypedItem that does not have Typelib type or is Port/Property");
    }
    std::vector<Conversion> typeCandidates;

    if (!typelibType && ti->type() != ItemType::OUTPUTPORT &&
            ti->type() != ItemType::INPUTPORT && ti->type() != ItemType::PROPERTYITEM)
        return typeCandidates;


    typeCandidates.push_back(Conversion(typeName, typelibType, nullptr,
                                        outputport, inputport, property,
                                        taskContextProxy, nameService,
                                        registry, fieldName, ti));

    auto eqrange = typeconverters.equal_range(typeName);
    for (auto it =  eqrange.first; it != eqrange.second; it++)
    {
        if (registry)
        {
            typeCandidates.push_back(
                Conversion(it->second->getResultTypename(),
                           registry->get(it->second->getResultTypename()),
                           it->second, outputport, inputport, property,
                           taskContextProxy, nameService, registry, fieldName,
                           ti));
        }
        else
        {
            typeCandidates.push_back(
                Conversion(it->second->getResultTypename(), nullptr,
                           it->second, outputport, inputport, property,
                           taskContextProxy, nameService, registry, fieldName,
                           ti));
        }
    }
    return typeCandidates;
}

std::vector<FieldVizHandle *> MainWindow::findWidgetsShowingItem(TypedItem *ti)
{
    //iterates over all widgets, finding ones that have ti

    std::vector< FieldVizHandle*> result;

    //find widgets that already show this ti,
    //find widgets that could potentially show this ti, remove those that already do
    for (auto const &i : fieldVizHandles)
    {
        if (i.first != ti)
            continue;
        result.push_back(i.second);
    }

    return result;
}

std::vector<MainWindow::AddFieldInfo> MainWindow::findWidgetsThatCanShowItemButDontForTheSubplugin(TypedItem *ti) {
    //iterates over all widgets, finding ones that support ti, resulting in
    //a list of subplugins

    std::vector<Conversion > typeCandidates = createTypeCandidates(typeconverters, ti);

    std::vector<MainWindow::AddFieldInfo> result;

    for (auto const &w : widgets)
    {
        auto p = w.plugin;
        if (!(p->getFlags() & rockdisplay::vizkitplugin::Plugin::WidgetCanHandleMultipleFields))
            continue;
        if (p->getFlags() & rockdisplay::vizkitplugin::Plugin::SingleFieldOnly)
            continue;

        std::unordered_set<std::string> seen_subplugins;
        for (auto &tc : typeCandidates)
        {
            std::vector<std::string> subpluginNames;
            bool supported = false;
            if (tc.outputportFieldDesc)
            {
                supported = p->probeOutputPort(
                                tc.outputportFieldDesc.get(), subpluginNames
                            );
            }
            if (tc.inputportFieldDesc)
            {
                supported = p->probeInputPort(
                                tc.inputportFieldDesc.get(), subpluginNames
                            );
            }
            if (tc.propertyFieldDesc)
            {
                supported = p->probeProperty(
                                tc.propertyFieldDesc.get(), subpluginNames
                            );
            }
            if (supported)
            {
                //yes, it supports this ti.
                AddFieldInfo ai;
                ai.plugin = p;
                ai.widget = w.widget;
                ai.converterfactory = tc.converterfactory;
                if (subpluginNames.empty())
                    subpluginNames.push_back(std::string());
                for (auto const &n : subpluginNames)
                {
                    if (seen_subplugins.find(n) != seen_subplugins.end())
                        continue;
                    seen_subplugins.insert(n);
                    ai.subpluginname = n;
                    if (!(p->getFlags() & rockdisplay::vizkitplugin::
                            Plugin::AllowDuplicateFields))
                    {
                        bool existsAlreadyForSubplugin = false;
                        for (auto const &i : fieldVizHandles)
                        {
                            if (i.first == ti &&
                                    i.second->plugin == p &&
                                    i.second->widget == w.widget &&
                                    i.second->subpluginname == n)
                            {
                                existsAlreadyForSubplugin = true;
                                break;
                            }
                        }
                        if (existsAlreadyForSubplugin)
                            continue;
                    }
                    result.push_back(ai);
                }
            }
        }
    }

    return result;
}

std::vector<MainWindow::AddWidgetInfo>
MainWindow::findPluginsThatCanCreateWidgetShowingItem(TypedItem *ti)
{
    //iterates over all plugins, finding the ones that support ti, resulting in
    //a list of subplugins
    //find plugins that will handle this ti, clear their widget lists to ensure they exist in the map

    std::vector<MainWindow::AddWidgetInfo> result;

    std::vector<Conversion> typeCandidates = createTypeCandidates(typeconverters , ti);

    for (rockdisplay::vizkitplugin::Plugin *p : plugins)
    {
        if (p->getFlags() & rockdisplay::vizkitplugin::Plugin::Flags::PreferSingleWidget)
        {
            bool found = false;
            for (auto &w : widgets)
            {
                if (w.plugin == p)
                {
                    found = true;
                    break;
                }
            }
            if(found)
                continue;
        }
        AddWidgetInfo ai;
        ai.plugin = p;
        std::unordered_set<std::string> seen_subplugins;
        for (auto &tc : typeCandidates)
        {
            std::vector<std::string> subpluginNames;
            if (tc.outputportFieldDesc && p->probeOutputPort(tc.outputportFieldDesc.get(), subpluginNames))
            {
                if (subpluginNames.empty())
                    subpluginNames.push_back(std::string());
            }
            if (tc.inputportFieldDesc && p->probeInputPort(tc.inputportFieldDesc.get(), subpluginNames))
            {
                if (subpluginNames.empty())
                    subpluginNames.push_back(std::string());
            }
            if (tc.propertyFieldDesc && p->probeProperty(tc.propertyFieldDesc.get(), subpluginNames))
            {
                if (subpluginNames.empty())
                    subpluginNames.push_back(std::string());
            }
            ai.converterfactory = tc.converterfactory;
            for (auto const &n : subpluginNames)
            {
                if (seen_subplugins.find(n) != seen_subplugins.end())
                    continue;
                seen_subplugins.insert(n);
                ai.subpluginname = n;
                result.push_back(ai);
            }
        }
    }

    return result;
}

std::unordered_map<rockdisplay::vizkitplugin::Widget*, unsigned int> MainWindow::calculateWidgetUsageCount()
{
    //iterates over all widgets, calculating the number of fields added to each

    std::unordered_map<rockdisplay::vizkitplugin::Widget*, unsigned int> result;

    for (auto const &i : fieldVizHandles)
    {
        auto it = result.find(i.second->widget);
        if (it == result.end())
            result[i.second->widget] = 1;
        else
            result[i.second->widget]++;
    }
    for (auto const &i : standalonePluginHandles)
    {
        auto it = result.find(i->widget.widget);
        if(it == result.end())
            result[i->widget.widget] = 1;
        else
            result[i->widget.widget]++;
    }

    return result;
}

bool MainWindow::populatePluginMenuSection(QMenu *menu, TypedItem *ti)
{
    bool have_entries = false;

    std::vector<FieldVizHandle *> removables = findWidgetsShowingItem(ti);
    std::vector<MainWindow::AddFieldInfo> addablefields =
        findWidgetsThatCanShowItemButDontForTheSubplugin(ti);
    std::vector<MainWindow::AddWidgetInfo> addablewidgets =
        findPluginsThatCanCreateWidgetShowingItem(ti);
    std::unordered_map<rockdisplay::vizkitplugin::Widget *, unsigned int> widgetUsageCount =
        calculateWidgetUsageCount();

    struct PluginInfo
    {
        rockdisplay::vizkitplugin::Plugin *plugin;
        struct SubpluginInfo
        {
            std::string subpluginname;
            bool widgetAddable;
            std::vector<rockdisplay::vizkitplugin::Widget *> fieldsAddable;
            std::vector<FieldVizHandle *> removables;
            TypeConverterFactory *converterfactory;
            SubpluginInfo() :widgetAddable(false) {}
        };
        std::unordered_map<std::string, SubpluginInfo> subplugins;
        std::vector < SubpluginInfo > ordered_subplugins;
    };

    std::unordered_map < rockdisplay::vizkitplugin::Plugin *, PluginInfo > plugins;
    for (auto &r : removables)
    {
        plugins[r->plugin].subplugins[r->subpluginname].removables.push_back(r);
    }
    for (auto &af : addablefields)
    {
        plugins[af.plugin].subplugins[af.subpluginname].fieldsAddable.push_back(af.widget);
        plugins[af.plugin].subplugins[af.subpluginname].converterfactory = af.converterfactory;
    }
    for (auto &aw : addablewidgets)
    {
        plugins[aw.plugin].subplugins[aw.subpluginname].widgetAddable = true;
        plugins[aw.plugin].subplugins[aw.subpluginname].converterfactory = aw.converterfactory;
    }
    std::vector < PluginInfo > ordered_plugins;
    for (auto &p : plugins)
    {
        p.second.plugin = p.first;
        ordered_plugins.push_back(p.second);
    }

    std::sort(ordered_plugins.begin(), ordered_plugins.end(), [](
                  PluginInfo const & a,
                  PluginInfo const & b
              ) -> bool
    {
        std::string const &_a = a.plugin->getName();
        std::string const &_b = b.plugin->getName();
        return std::lexicographical_compare(_a.begin(), _a.end(),
                                            _b.begin(), _b.end());
    });

    for (auto &p : ordered_plugins)
    {
        for (auto &sp : p.subplugins)
        {
            sp.second.subpluginname = sp.first;
            p.ordered_subplugins.push_back(sp.second);
        }
        std::sort(p.ordered_subplugins.begin(), p.ordered_subplugins.end(), [](
                      PluginInfo::SubpluginInfo const & a,
                      PluginInfo::SubpluginInfo const & b
                  ) -> bool
        {
            std::string const &_a = a.subpluginname;
            std::string const &_b = b.subpluginname;
            return std::lexicographical_compare(_a.begin(), _a.end(),
                                                _b.begin(), _b.end());
        });
    }

    for (auto &p : ordered_plugins)
    {
        std::vector<PluginInfo::SubpluginInfo> subplugins = p.ordered_subplugins;
        if(subplugins.empty())
        {
            PluginInfo::SubpluginInfo spi;
            spi.subpluginname = "";
            subplugins.push_back(spi);
        }
        for (auto &subplugin : subplugins)
        {
            struct SubpluginActions {
                QAction *directAction;
                QAction *submenuAction;
                SubpluginActions() : directAction(nullptr), submenuAction(nullptr) {}
            };

            //if there is only one action, we will add it directly to the menu.
            //otherwise, we add a sub-menu where all the actions reside.

            //for this, collect all available actions first.
            //we will directly dispose of half of them, or, may not even bother
            //to populate the directAction when there is already more than one.
            std::vector<SubpluginActions> actions;

            rockdisplay::vizkitplugin::Plugin *plugin = p.plugin;
            std::string subpluginname = subplugin.subpluginname;
            TypeConverterFactory *converterfactory = subplugin.converterfactory;
            QString directPluginName = QString::fromStdString(p.plugin->getName());
            if (!p.ordered_subplugins.empty())
            {
                if (subpluginname.empty())
                {
                    directPluginName = QString::fromStdString(p.plugin->getName());
                }
                else
                {
                    directPluginName = tr("%1: %2", "Plugin: Subplugin").
                                       arg(QString::fromStdString(p.plugin->getName())).
                                       arg(QString::fromStdString(subpluginname));
                }
            }


            //do we offer to create a new widget?
            if (subplugin.widgetAddable)
            {
                SubpluginActions a;
                a.submenuAction = new QAction(tr("Create new widget"));
                connect(a.submenuAction, &QAction::triggered,
                        this, [this, ti, plugin, subpluginname, converterfactory]()
                {
                    rockdisplay::vizkitplugin::Widget *widget = createWidget(plugin);
                    if(widget)
                    {
                        addFieldToWidget(plugin, subpluginname, widget, ti, converterfactory);
                    }
                });
                if(actions.empty()) {
                    //if actions already contains elements, don't bother creating the
                    //directAction, it will not be used
                    a.directAction = new QAction(directPluginName);
                    connect(a.directAction, &QAction::triggered,
                            this, [this, ti, plugin, subpluginname, converterfactory]()
                    {
                        rockdisplay::vizkitplugin::Widget *widget = createWidget(plugin);
                        if (widget)
                        {
                            addFieldToWidget(plugin, subpluginname, widget, ti, converterfactory);
                        }
                    });
                }
                actions.push_back(a);
            }
            //add add-to entries
            for (auto &w : subplugin.fieldsAddable)
            {
                SubpluginActions a;
                a.submenuAction = new QAction(tr("Add to %1").arg(widgetWindows[w]->windowTitle()));
                rockdisplay::vizkitplugin::Widget *widget = w;
                connect(a.submenuAction, &QAction::triggered,
                        this, [this, ti, plugin, widget, subpluginname, converterfactory]()
                {
                    addFieldToWidget(plugin, subpluginname, widget, ti, converterfactory);
                });
                if(actions.empty()) {
                    //if actions already contains elements, don't bother creating the
                    //directAction, it will not be used
                    a.directAction = new QAction(directPluginName);
                    connect(a.directAction, &QAction::triggered,
                            this, [this, ti, plugin, widget, subpluginname, converterfactory]()
                    {
                        addFieldToWidget(plugin, subpluginname, widget, ti, converterfactory);
                    });
                }
                actions.push_back(a);
            }
            //remove and remove-from
            for (auto &fvh : subplugin.removables)
            {
                SubpluginActions a;
                rockdisplay::vizkitplugin::Widget *widget = fvh->widget;
                if ((p.plugin->getFlags() &
                        rockdisplay::vizkitplugin::Plugin::Flags::CanRemoveFields) == 0 ||
                        ((p.plugin->getFlags() & rockdisplay::vizkitplugin::Plugin::Flags::KeepOpenWithoutFields) == 0 &&
                        widgetUsageCount[widget] <= 1))
                {
                    a.submenuAction = new QAction(tr("Remove %1").arg(widgetWindows[widget]->windowTitle()));
                    connect(a.submenuAction, &QAction::triggered,
                            this, [this, plugin, widget]()
                    {
                        destroyWidget(plugin, widget);
                    });
                    if (actions.empty())
                    {
                        //if actions already contains elements, don't bother creating the
                        //directAction, it will not be used
                        a.directAction = new QAction(tr("Remove %1").arg(directPluginName));
                        connect(a.directAction, &QAction::triggered,
                                this, [this, plugin, widget]()
                        {
                            destroyWidget(plugin, widget);
                        });
                    }
                }
                else
                {
                    a.submenuAction = new QAction(tr("Remove from %1").arg(widgetWindows[widget]->windowTitle()));
                    connect(a.submenuAction, &QAction::triggered,
                            this, [this, ti, fvh]()
                    {
                        removeFieldFromWidget(fvh, ti);
                    });
                    if (actions.empty())
                    {
                        //if actions already contains elements, don't bother creating the
                        //directAction, it will not be used
                        a.directAction = new QAction(tr("Remove from %1").arg(directPluginName));
                        connect(a.directAction, &QAction::triggered,
                                this, [this, ti, fvh]()
                        {
                            removeFieldFromWidget(fvh, ti);
                        });
                    }
                }
                actions.push_back(a);
            }

            if(actions.size() == 0)
                continue;

            have_entries = true;

            if (actions.size() == 1)
            {
                for (auto &a : actions)
                {
                    a.directAction->setParent(menu);
                    menu->addAction(a.directAction);
                    delete a.submenuAction;
                }
                continue;
            }

            QMenu *submenu = menu->addMenu(directPluginName);

            for (auto &a : actions)
            {
                a.submenuAction->setParent(submenu);
                submenu->addAction(a.submenuAction);
                if(a.directAction)
                    delete a.directAction;
            }

        }
    }

    return have_entries;
}

void MainWindow::prepareMenu(const QPoint &pos)
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
                TaskItem *titem = static_cast<TaskItem *>(ti->getData());

                RTT::corba::TaskContextProxy* task = titem->getTaskContext();

                menu->addAction(labelAction);
                label->setText(tr("<b>Actions</b>"));
                QAction *act = menu->addAction("Activate");
                QAction *sta = menu->addAction("Start");
                QAction *sto = menu->addAction("Stop");
                QAction *con = menu->addAction("Configure");
                QAction *clu = menu->addAction("Cleanup");
                QAction *load = menu->addAction(tr("Load Configuration from file..."));
                QAction *save = menu->addAction(tr("Save Configuration to file..."));

                connect(act, &QAction::triggered,
                        this, [task]()
                {
                    task->activate();
                });
                connect(sta, &QAction::triggered,
                        this, [task]()
                {
                    task->start();
                });
                connect(sto, &QAction::triggered,
                        this, [task]()
                {
                    task->stop();
                });
                connect(con, &QAction::triggered,
                        this, [task]()
                {
                    task->configure();
                });
                connect(clu, &QAction::triggered,
                        this, [task]()
                {
                    task->cleanup();
                });

                connect(load, &QAction::triggered,
                    this, [this,titem,task](){
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
                    this, [this,titem,task](){
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
            case ItemType::PROPERTYITEM:
            {
                ItemBase* itembase = nullptr;
                if(ti->type() == ItemType::CONFIGITEM || ti->type() == ItemType::EDITABLEITEM)
                {
                    itembase = static_cast<ItemBase*>(ti->getData());
                }
                else if (ti->type() == ItemType::INPUTPORT || ti->type() == ItemType::OUTPUTPORT)
                {
                    itembase = static_cast<PortItem *>(ti->getData())->getItemBase().get();
                }
                else if (ti->type() == ItemType::PROPERTYITEM)
                {
                    itembase = static_cast<PropertyItem *>(ti->getData())->getItemBase().get();
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
                
                menu->addAction(labelAction);
                label->setText(tr("<b>Plugins</b>"));

                bool have_plugins = populatePluginMenuSection(menu, ti);

                if (!have_plugins)
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

rockdisplay::vizkitplugin::Widget *MainWindow::createWidget(rockdisplay::vizkitplugin::Plugin *plugin)
{
    rockdisplay::vizkitplugin::Widget *widget = nullptr;
    PluginWidgetQMainWindow *w = nullptr;
    //do we have some widgets created already that have been kept around due to KeepWidget?
    if (hiddenWidgets.find(plugin) != hiddenWidgets.end() &&
            !hiddenWidgets[plugin].empty())
    {
        widget = hiddenWidgets[plugin].back().first;
        w = hiddenWidgets[plugin].back().second;
        hiddenWidgets[plugin].pop_back();
    }
    else
    {
        widget = plugin->createWidget();
        if (!widget)
        {
            return nullptr;
        }
        w = new PluginWidgetQMainWindow;
        w->setCentralWidget(widget->getWidget());

        connect(w, &PluginWidgetQMainWindow::closing,
                this, [this, plugin, widget]()
        {
            //this either deletes widget or at least hides it and pretends it was deleted
            destroyWidget(plugin, widget);
        });
    }
    WidgetInfo i{widget, plugin};
    widgets.push_back(i);
    QString title = QString::fromStdString(plugin->getName());
    bool plainTitleFound = false;
    for (auto &w : widgetWindows)
    {
        printf("Found window with title %s\n",w.second->windowTitle().toLocal8Bit().data());
        if (w.second->windowTitle() == title)
        {
            plainTitleFound = true;
            break;
        }
    }
    if(plainTitleFound)
    {
        int num = 1;
        bool titleFound;
        do {
            title = tr("%1-%2","PluginWindowTitle-Number").arg(QString::fromStdString(plugin->getName())).arg(num);
            titleFound = false;
            for (auto &w : widgetWindows)
            {
                if (w.second->windowTitle() == title)
                {
                    titleFound = true;
                    break;
                }
            }
            if(titleFound)
            {
                num++;
            }
        } while(titleFound);
    }
    w->setWindowTitle(title);
    printf("Using title %s\n",title.toLocal8Bit().data());
    widgetWindows[widget] = w;

    w->show();

    regenerateWidgetsMenu();

    return widget;
}

bool MainWindow::addFieldToWidget(rockdisplay::vizkitplugin::Plugin *plugin, std::string const &subpluginname, rockdisplay::vizkitplugin::Widget *widget, TypedItem *ti, TypeConverterFactory *converterfactory)
{
    bool haveAdded = false;

    if(!ti) {
        StandalonePluginHandle *vh = new StandalonePluginHandle();
        vh->widget.widget = widget;
        vh->widget.plugin = plugin;
        vh->subpluginname = subpluginname;
        vh->standalonesubplugin = widget->addStandaloneSubplugin(subpluginname);
        if (vh->standalonesubplugin)
        {
            standalonePluginHandles.insert(vh);
            haveAdded = true;
        }
        else
        {
            delete vh;
        }

        return haveAdded;
    }

    VisualizerAdapter *viz = static_cast<VisualizerAdapter *>(ti->getData());
    ItemInfo iteminfo = getItemInfo(ti);
    auto outputport = iteminfo.outputPort;
    auto inputport = iteminfo.inputPort;
    auto property = iteminfo.property;
    RTT::corba::TaskContextProxy *taskContextProxy = iteminfo.taskContextProxy;
    orocos_cpp::NameService *nameService = iteminfo.nameService;
    assert(nameService);
    Typelib::Type const *typelibType = iteminfo.typelibType;
    Typelib::Registry const *registry = iteminfo.registry;
    std::string fieldName = iteminfo.fieldName;

    FieldVizHandle *vh = new FieldVizHandle();
    vh->item = ti;
    vh->widget = widget;
    vh->plugin = plugin;
    vh->converterfactory = converterfactory;
    vh->subpluginname = subpluginname;
    if (outputport)
    {
        if (converterfactory)
        {
            vh->fieldHandle = new rockdisplay::XlatingFieldDescriptionImpl
            (outputport, taskContextProxy, nameService, typelibType, registry, fieldName, ti, converterfactory, vh);
            printf("Setting up converting visualizer");
        }
        else
        {
            vh->fieldHandle = new rockdisplay::FieldDescriptionImpl
            (outputport, taskContextProxy, nameService, typelibType, registry, fieldName, ti, vh);
        }
        vh->outputportfield = widget->addOutputPortField(vh->fieldHandle, subpluginname);
        if (vh->outputportfield)
        {
            viz->addPlugin(plugin->getName()+subpluginname, vh);
            fieldVizHandles.insert(std::make_pair(ti, vh));
            //send the current, old, data if any to everything, including this new visualizer
            model->updateTasks(true);
            haveAdded = true;
        }
        else
        {
            printf("Rejected by plugin(type %s)\n",vh->fieldHandle->getTypeName().c_str());
            delete vh;
        }
    }
    if (inputport)
    {
        if (converterfactory)
        {
            vh->fieldHandle = new rockdisplay::XlatingFieldDescriptionImpl
            (inputport, taskContextProxy, nameService, typelibType, registry, fieldName, ti, converterfactory, vh);
        }
        else
        {
            vh->fieldHandle = new rockdisplay::FieldDescriptionImpl
            (inputport, taskContextProxy, nameService, typelibType, registry, fieldName, ti, vh);
        }
        vh->inputportfield = widget->addInputPortField(vh->fieldHandle, subpluginname);
        if (vh->inputportfield)
        {
            viz->addPlugin(plugin->getName()+subpluginname, vh);
            fieldVizHandles.insert(std::make_pair(ti, vh));
            connect(vh, &FieldVizHandle::editableChanged,
                    this, [this, ti](const Typelib::Value& value, bool force_send)
            {
                (void)value;
                this->itemDataEdited(ti, force_send);
            });
            //send the current, old, data if any to everything, including this new visualizer
            model->updateTasks(true);
            haveAdded = true;
        }
        else
        {
            delete vh;
        }
    }
    if (property)
    {
        if (converterfactory)
        {
            vh->fieldHandle = new rockdisplay::XlatingFieldDescriptionImpl
            (property, taskContextProxy, nameService, typelibType, registry, fieldName, ti, converterfactory, vh);
        }
        else
        {
            vh->fieldHandle = new rockdisplay::FieldDescriptionImpl
            (property, taskContextProxy, nameService, typelibType, registry, fieldName, ti, vh);
        }
        vh->propertyfield = widget->addPropertyField(vh->fieldHandle, subpluginname);
        if (vh->propertyfield)
        {
            viz->addPlugin(plugin->getName()+subpluginname, vh);
            fieldVizHandles.insert(std::make_pair(ti, vh));
            connect(vh, &FieldVizHandle::editableChanged,
                    this, [this, ti](const Typelib::Value & value, bool forceSend)
            {
                (void)value;
                this->itemDataEdited(ti, forceSend);
            });
            //send the current, old, data if any to everything, including this new visualizer
            model->updateTasks(true);
            haveAdded = true;
        }
        else
        {
            delete vh;
        }
    }
    return haveAdded;
}

void MainWindow::destroyWidget(rockdisplay::vizkitplugin::Plugin *plugin, rockdisplay::vizkitplugin::Widget *widget)
{
    //we must deregister its VizHandle from all associated VisualizerAdapters(can be derived from the TypedItem)
    //at the same time, remove it from our internal structures
    for (auto it = fieldVizHandles.begin();
            it != fieldVizHandles.end();)
    {
        if (it->second->widget == widget)
        {
            auto vh = it->second;
            it = fieldVizHandles.erase(it);
            VisualizerAdapter *viz = static_cast<ItemBase *>(vh->item->getData());
            viz->removeVisualizer(vh);
            if (vh->outputportfield)
            {
                widget->removeOutputPortField(vh->fieldHandle, vh->outputportfield);
            }
            if (vh->inputportfield)
            {
                widget->removeInputPortField(vh->fieldHandle, vh->inputportfield);
            }
            if (vh->propertyfield)
            {
                widget->removePropertyField(vh->fieldHandle, vh->propertyfield);
            }
            delete vh;
        }
        else
        {
            it++;
        }
    }
    for (auto it = standalonePluginHandles.begin();
            it != standalonePluginHandles.end();)
    {
        if ((*it)->widget.widget == widget)
        {
            auto vh = *it;
            it = standalonePluginHandles.erase(it);
            widget->removeStandaloneSubplugin(vh->standalonesubplugin);
            delete vh;
        }
        else
        {
            it++;
        }
    }
    //finally remove from newStyleWidgets
    for (auto it = widgets.begin();
            it != widgets.end();)
    {
        if (it->widget == widget)
        {
            it = widgets.erase(it);
        }
        else
        {
            it++;
        }
    }
    if ((plugin->getFlags() & rockdisplay::vizkitplugin::Plugin::KeepWidgets) != 0)
    {
        auto winit = widgetWindows.find(widget);
        PluginWidgetQMainWindow *win = nullptr;
        if (winit != widgetWindows.end())
        {
            win = winit->second;
            widgetWindows.erase(winit);
        }
        hiddenWidgets[plugin].push_back(std::make_pair(widget, win));
    }
    else
    {
        auto winit = widgetWindows.find(widget);
        if (winit != widgetWindows.end())
        {
            auto win = winit->second;
            widgetWindows.erase(winit);
            win->deleteLater();
        }
        //and delete it
        delete widget;
    }

    regenerateWidgetsMenu();

}

void MainWindow::removeFieldFromWidget(FieldVizHandle *fvh, TypedItem *ti)
{
    VisualizerAdapter *viz = static_cast<ItemBase *>(ti->getData());
    for (auto fit = fieldVizHandles.equal_range(ti);
            fit.first != fit.second; fit.first++)
    {
        if (fit.first->second != fvh)
        {
            continue;
        }
        fieldVizHandles.erase(fit.first);
        viz->removeVisualizer(fvh);
        if (fvh->outputportfield)
        {
            fvh->widget->removeOutputPortField(fvh->fieldHandle, fvh->outputportfield);
        }
        if (fvh->inputportfield)
        {
            fvh->widget->removeInputPortField(fvh->fieldHandle, fvh->inputportfield);
        }
        if (fvh->propertyfield)
        {
            fvh->widget->removePropertyField(fvh->fieldHandle, fvh->propertyfield);
        }
        delete fvh;
        break;
    }
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



#ifdef HAVE_TRANSFORMER_TYPEKIT
#include <transformer/BroadcastTypes.hpp>
#include <functional>
#include <string>

namespace std
{
template<>
struct hash<transformer::PortTransformationAssociation>
{
    std::size_t operator()(transformer::PortTransformationAssociation const &pa) const noexcept
    {
        std::size_t h1 = std::hash<std::string> {}(pa.task);
        std::size_t h2 = std::hash<std::string> {}(pa.port);
        std::size_t h3 = std::hash<std::string> {}(pa.from_frame);
        std::size_t h4 = std::hash<std::string> {}(pa.to_frame);
        return h1 ^ (h2 << 1) ^ (h3 << 2) ^ (h4 << 3);
    }
};
}

//must be first to avoid qt #defined emit, signal
#include <rtt/TaskContext.hpp>
#endif

#include "vizkit3dplugins.hpp"
#include "vizkit3dplugins_p.hpp"

#include "Vizkit3dPluginRepository.hpp"
#include <QApplication>
#include <typelib/value.hh>
#include <vizkit3d/Vizkit3DWidget.hpp>

#ifdef HAVE_TRANSFORMER_TYPEKIT
#include <orocos_cpp_base/OrocosHelpers.hpp>
#include <orocos_cpp/NameService.hpp>
#include <rtt/InputPort.hpp>
#include <rtt/base/OutputPortInterface.hpp>
#include <rtt/transports/corba/TaskContextProxy.hpp>
#endif

using namespace rock_display;

Vizkit3DPluginsWidget::TransformerData::TransformerData()
    : broadcaster_port(nullptr), broadcaster_reader(nullptr),
      broadcaster_task(nullptr)
{
}


Vizkit3dPluginsOutputPortField::Vizkit3dPluginsOutputPortField(QObject *plugin, QMetaMethod method)
    : plugin(plugin), method(method)
{
}

void Vizkit3dPluginsOutputPortField::updateOutputPort(const rockdisplay::vizkitplugin::ValueHandle *value)
{
    QGenericArgument val("void *", value->getRawPtr());
    if (!val.data())
    {
        return;
    }

    method.invoke(plugin, val);
}

Vizkit3dPluginsInputPortField::Vizkit3dPluginsInputPortField(QObject *plugin, QMetaMethod method)
    : plugin(plugin), method(method)
{
}

void Vizkit3dPluginsInputPortField::updateInputPort(rockdisplay::vizkitplugin::ValueHandle *value)
{
    QGenericArgument val("void *", value->getRawPtr());
    if (!val.data())
    {
        return;
    }

    method.invoke(plugin, val);
}

Vizkit3dPluginsPropertyField::Vizkit3dPluginsPropertyField(QObject *plugin, QMetaMethod method)
    : plugin(plugin), method(method)
{
}

void Vizkit3dPluginsPropertyField::updateProperty(rockdisplay::vizkitplugin::ValueHandle *value)
{
    QGenericArgument val("void *", value->getRawPtr());
    if (!val.data())
    {
        return;
    }

    method.invoke(plugin, val);
}

Vizkit3dPluginsStandaloneSubplugin::Vizkit3dPluginsStandaloneSubplugin(QObject *plugin)
    : plugin(plugin)
{
}

#ifdef HAVE_TRANSFORMER_TYPEKIT

Vizkit3dPluginsTransformerDispatchOutputPortField::Vizkit3dPluginsTransformerDispatchOutputPortField(Vizkit3DPluginsWidget *widget)
    : widget(widget)
{
}

static std::string getFreePortName(RTT::TaskContext* clientTask, const RTT::base::PortInterface* portIf)
{
    int cnt = 0;
    while(true)
    {
        std::string localName = QString("%1%2").arg(QString::fromStdString(portIf->getName())).arg(cnt).toStdString();
        if(clientTask->getPort(localName))
        {
            cnt++;
        }
        else
        {
            return localName;
        }
    }
}

void Vizkit3dPluginsTransformerDispatchOutputPortField::updateOutputPort(const rockdisplay::vizkitplugin::ValueHandle *value)
{
    if(value->getFieldDescription()->getTypeName() == "/transformer/ConfigurationState") {
        auto state = reinterpret_cast<transformer::ConfigurationState const *>(value->getRawPtr());
        widget->push_transformer_configuration(state, value->getFieldDescription()->getNameService());
    }
    if (value->getFieldDescription()->getTypeName() == "/base/samples/RigidBodyState")
    {
        auto trsf = reinterpret_cast<base::samples::RigidBodyState const *>(value->getRawPtr());
        widget->push_rigidbodystate(trsf, value->getFieldDescription()->getNameService());
    }
}

#endif

Vizkit3DPluginsWidget::Vizkit3DPluginsWidget(Vizkit3dPluginRepository *pluginRepo,
        vizkit3d::Vizkit3DWidget *v3dwidget)
    : v3dwidget(v3dwidget), pluginRepo(pluginRepo)
#ifdef HAVE_TRANSFORMER_TYPEKIT
    , rttQueryTimer(nullptr)
#endif
{
}

Vizkit3DPluginsWidget::~Vizkit3DPluginsWidget() {
    emit deleting();
}


QWidget *Vizkit3DPluginsWidget::getWidget()
{
    return v3dwidget;
}

rockdisplay::vizkitplugin::Field *Vizkit3DPluginsWidget::addOutputPortField(const rockdisplay::vizkitplugin::FieldDescription *type, std::string const &subpluginname)
{
#ifdef HAVE_TRANSFORMER_TYPEKIT
    check_transformer_broadcaster_listener(type->getNameService());

    if(subpluginname == "TransformerDispatch")
    {
        return new Vizkit3dPluginsTransformerDispatchOutputPortField(this);
    }
#endif

    auto list = pluginRepo->getPluginsForType(type->getTypeName(), type->getRegistry());
    for(auto & e : list) {
        if (e->pluginName == subpluginname) {
            QObject *plugin = e->factory->createPlugin(QString::fromStdString(e->pluginName));
            QMetaMethod method = e->method;

            if (!plugin)
            {
                continue;
            }

            v3dwidget->addPlugin(plugin);
            v3dwidget->show();

            return new Vizkit3dPluginsOutputPortField(plugin, method);
        }
    }
    return nullptr;
}

rockdisplay::vizkitplugin::Field *Vizkit3DPluginsWidget::addInputPortField(const rockdisplay::vizkitplugin::FieldDescription *type, std::string const &subpluginname)
{
    auto list = pluginRepo->getPluginsForType(type->getTypeName(), type->getRegistry());
    for(auto & e : list) {
        if (e->pluginName == subpluginname) {
            QObject *plugin = e->factory->createPlugin(QString::fromStdString(e->pluginName));
            QMetaMethod method = e->method;

            if (!plugin)
            {
                continue;
            }

            v3dwidget->addPlugin(plugin);
            v3dwidget->show();

            return new Vizkit3dPluginsInputPortField(plugin, method);
        }
    }
    return nullptr;
}

rockdisplay::vizkitplugin::Field *Vizkit3DPluginsWidget::addPropertyField(const rockdisplay::vizkitplugin::FieldDescription *type, std::string const &subpluginname)
{
    auto list = pluginRepo->getPluginsForType(type->getTypeName(), type->getRegistry());
    for(auto & e : list) {
        if (e->pluginName == subpluginname) {
            QObject *plugin = e->factory->createPlugin(QString::fromStdString(e->pluginName));
            QMetaMethod method = e->method;

            if (!plugin)
            {
                continue;
            }

            v3dwidget->addPlugin(plugin);
            v3dwidget->show();

            return new Vizkit3dPluginsPropertyField(plugin, method);
        }
    }
    return nullptr;
}

rockdisplay::vizkitplugin::StandaloneSubplugin *Vizkit3DPluginsWidget::addStandaloneSubplugin(
        std::string const &subpluginname) {
    auto list = pluginRepo->getAllAvailablePlugins();
    for(auto & e : list) {
        if (e->pluginName == subpluginname) {

            QObject *plugin = e->factory->createPlugin(QString::fromStdString(e->pluginName));

            if (!plugin)
            {
                continue;
            }

            v3dwidget->addPlugin(plugin);
            v3dwidget->show();

            return new Vizkit3dPluginsStandaloneSubplugin(plugin);
        }
    }
    return nullptr;
}

void Vizkit3DPluginsWidget::removeStandaloneSubplugin(rockdisplay::vizkitplugin::StandaloneSubplugin *f) {
}

void Vizkit3DPluginsWidget::removeInputPortField(rockdisplay::vizkitplugin::FieldDescription const *field,
        rockdisplay::vizkitplugin::Field *f)
{
    auto our_f = static_cast<Vizkit3dPluginsInputPortField*>(f);
    v3dwidget->removePlugin(our_f->plugin);
    delete f;
}

void Vizkit3DPluginsWidget::removeOutputPortField(rockdisplay::vizkitplugin::FieldDescription const *field,
        rockdisplay::vizkitplugin::Field *f)
{

    auto plugin_f = dynamic_cast<Vizkit3dPluginsOutputPortField*>(f);
#ifdef HAVE_TRANSFORMER_TYPEKIT
    //this cast must happen before potential deletion because deletion
    //destroys the type information.
    auto transform_disp_f = dynamic_cast<Vizkit3dPluginsTransformerDispatchOutputPortField*>(f);
#endif
    if (plugin_f)
    {
        v3dwidget->removePlugin(plugin_f->plugin);
        delete f;
    }
#ifdef HAVE_TRANSFORMER_TYPEKIT
    if (transform_disp_f)
    {
        delete f;
    }
#endif
}

void Vizkit3DPluginsWidget::removePropertyField(rockdisplay::vizkitplugin::FieldDescription const *field,
        rockdisplay::vizkitplugin::Field *f)
{
    auto our_f = static_cast<Vizkit3dPluginsPropertyField*>(f);
    v3dwidget->removePlugin(our_f->plugin);
    delete f;
}

#ifdef HAVE_TRANSFORMER_TYPEKIT

void Vizkit3DPluginsWidget::check_transformer_broadcaster_listener(orocos_cpp::NameService *nameservice)
{
    if (transformerData.find(nameservice) != transformerData.end() &&
            transformerData[nameservice].broadcaster_reader != nullptr)
    {
        auto &td = transformerData[nameservice];
        if(td.broadcaster_task && (!td.broadcaster_task->server() || td.broadcaster_task->server()->_is_nil()))
        {
            //TODO this is never reached, even though from inspecting TaskModel and TaskItem it seemed to be the right thing.
            td.broadcaster_task = nullptr;
            td.broadcaster_port = nullptr;
            td.broadcaster_reader->disconnect();
        }
        if(!td.broadcaster_task)
        {
            if (!td.broadcaster_taskname.empty() && nameservice->isRegistered(td.broadcaster_taskname))
            {
                td.broadcaster_task = dynamic_cast<RTT::corba::TaskContextProxy*>(nameservice->getTaskContext(td.broadcaster_taskname));
                if(td.broadcaster_task)
                {
                    auto port = td.broadcaster_task->getPort("configuration_state");
                    if (port != td.broadcaster_port)
                    {
                        td.broadcaster_port = dynamic_cast<RTT::base::OutputPortInterface *>(port);
                        td.broadcaster_reader->disconnect();
                    }
                }
            }
        }
    }
    else
    {
        //list all tasks in the nameservice
        for (auto &taskname : nameservice->getRegisteredTasks())
        {
            //see if the task provides port "configuration_state" of type "/transformer/ConfigurationState"
            RTT::TaskContext *taskContext = nameservice->getTaskContext(taskname);
            auto port = taskContext->getPort("configuration_state");
            if (port && port->getTypeInfo()->getTypeName() == "/transformer/ConfigurationState")
            {
                //found a broadcaster

                TransformerData &td = transformerData[nameservice];

                td.broadcaster_taskname = taskname;
                td.broadcaster_port =
                    dynamic_cast<RTT::base::OutputPortInterface *>(port);
                td.broadcaster_reader = nullptr;
                if (td.broadcaster_port)
                {
                    td.broadcaster_reader =
                        dynamic_cast<RTT::InputPort<transformer::ConfigurationState> *>(
                            td.broadcaster_port->antiClone());
                    RTT::TaskContext *clientTask = OrocosHelpers::getClientTask();
                    td.broadcaster_reader->setName(
                        getFreePortName(clientTask, td.broadcaster_port));
                    clientTask->addPort(*td.broadcaster_reader);
                }

                setupRttQueryTimer();

                break;
            }
        }
    }
}

void Vizkit3DPluginsWidget::push_transformer_configuration(transformer::ConfigurationState const *state, orocos_cpp::NameService *nameservice)
{
    for (auto &trsf : state->static_transformations)
    {
        push_rigidbodystate(&trsf, nameservice);
    }
    for (auto &producer : state->port_transformation_associations)
    {
        if(transformerData[nameservice].ports.find(producer) ==
                transformerData[nameservice].ports.end())
        {
            transformerData[nameservice].ports[producer].task = nullptr;
            transformerData[nameservice].ports[producer].port = nullptr;
            transformerData[nameservice].ports[producer].reader = nullptr;
        }

        auto &pi = transformerData[nameservice].ports[producer];

        if (pi.task)
        {
            if(!pi.task->server() || pi.task->server()->_is_nil())
            {
                pi.task = nullptr;
                pi.port = nullptr;
                pi.reader->disconnect();
            }
        }

        if(!pi.task)
        {
            if (!nameservice->isRegistered(producer.task))
            {
                continue;
            }
            pi.task = dynamic_cast<RTT::corba::TaskContextProxy *>(nameservice->getTaskContext(producer.task));
            if (!pi.task)
            {
                continue;
            }
            RTT::base::OutputPortInterface *port = dynamic_cast<RTT::base::OutputPortInterface *>(pi.task->ports()->getPort(producer.port));
            if(port)
            {
                if (!pi.reader)
                {
                    pi.reader =
                        dynamic_cast<RTT::InputPort<base::samples::RigidBodyState> *>(port->antiClone());
                    RTT::TaskContext *clientTask = OrocosHelpers::getClientTask();
                    pi.reader->setName(
                        getFreePortName(clientTask, port));
                    clientTask->addPort(*pi.reader);
                }
                pi.port = port;
            }
            if (pi.port != port)
            {
                pi.port = port;
                pi.reader->disconnect();
            }
        }
    }

    setupRttQueryTimer();
}

void Vizkit3DPluginsWidget::push_rigidbodystate(base::samples::RigidBodyState const *trsf, orocos_cpp::NameService* nameservice) {
    if (trsf->targetFrame.empty() || trsf->sourceFrame.empty() ||
            trsf->targetFrame == trsf->sourceFrame)
    {
        return;
    }
    try {
        v3dwidget->setTransformation(QString::fromStdString(trsf->targetFrame),
                                     QString::fromStdString(trsf->sourceFrame),
                                     QVector3D(trsf->position[0],
                                               trsf->position[1],
                                               trsf->position[2]),
                                     QQuaternion(
                                         trsf->orientation.w(),
                                         trsf->orientation.x(),
                                         trsf->orientation.y(),
                                         trsf->orientation.z()));
    } catch(std::runtime_error const &e) {
    }
}
#endif

#if defined(HAVE_TRANSFORMER_TYPEKIT)
void Vizkit3DPluginsWidget::setupRttQueryTimer() {
    if (!rttQueryTimer)
    {
        rttQueryTimer = new QTimer(this);
        connect(rttQueryTimer, &QTimer::timeout,
                this, &Vizkit3DPluginsWidget::rttQueryTimeout);

        rttQueryTimer->start(100);
    }
}

void Vizkit3DPluginsWidget::rttQueryTimeout() {
#ifdef HAVE_TRANSFORMER_TYPEKIT
    for (auto &td : transformerData)
    {
        check_transformer_broadcaster_listener(td.first);
        if(td.second.broadcaster_reader) {
            if (!td.second.broadcaster_reader->connected())
            {
                RTT::ConnPolicy policy(RTT::ConnPolicy::data());
                policy.pull = true;
                td.second.broadcaster_reader->connectTo(
                    td.second.broadcaster_port, policy);
            }
            transformer::ConfigurationState state;
            if(td.second.broadcaster_reader->read(state) == RTT::NewData)
            {
                push_transformer_configuration(&state, td.first);
            }
        }
        for (auto &p : td.second.ports)
        {
            if (p.second.reader)
            {
                if (!p.second.reader->connected())
                {
                    RTT::ConnPolicy policy(RTT::ConnPolicy::data());
                    policy.pull = true;
                    p.second.reader->connectTo(
                        p.second.port, policy);
                }
                base::samples::RigidBodyState trsf;
                if (p.second.reader->read(trsf) == RTT::NewData)
                {
                    push_rigidbodystate(&trsf, td.first);
                }
            }
        }
    }
#endif
}
#endif

Vizkit3DPlugins::Vizkit3DPlugins()
: v3dwidget(new vizkit3d::Vizkit3DWidget), currentWidget(nullptr) {
    auto *list = v3dwidget->getAvailablePlugins();
    pluginRepo = new Vizkit3dPluginRepository(*list);
    delete list;
}

Vizkit3DPlugins::~Vizkit3DPlugins() {
    delete pluginRepo;
    v3dwidget->hide();
}

bool Vizkit3DPlugins::probeOutputPort(rockdisplay::vizkitplugin::FieldDescription *fieldDesc, std::vector<std::string> &names)
{

    auto list = pluginRepo->getPluginsForType(fieldDesc->getTypeName(), fieldDesc->getRegistry());
    for(auto &e : list) {
        names.push_back(e->pluginName);
    }

#ifdef HAVE_TRANSFORMER_TYPEKIT
    if (fieldDesc->getTypeName() == "/transformer/ConfigurationState" ||
        fieldDesc->getTypeName() == "/base/samples/RigidBodyState" ||
        false)
    {
        names.push_back("TransformerDispatch");
    }
#endif

    return names.size() != 0;
}

bool Vizkit3DPlugins::probeInputPort(rockdisplay::vizkitplugin::FieldDescription *fieldDesc, std::vector<std::string> &names)
{
    auto list = pluginRepo->getPluginsForType(fieldDesc->getTypeName(), fieldDesc->getRegistry());
    for(auto &e : list) {
        names.push_back(e->pluginName);
    }

    return names.size() != 0;
}

bool Vizkit3DPlugins::probeProperty(rockdisplay::vizkitplugin::FieldDescription *fieldDesc, std::vector<std::string> &names)
{
    auto list = pluginRepo->getPluginsForType(fieldDesc->getTypeName(), fieldDesc->getRegistry());
    for(auto &e : list) {
        names.push_back(e->pluginName);
    }

    return names.size() != 0;
}

rockdisplay::vizkitplugin::Widget *Vizkit3DPlugins::createWidget()
{
    //while Vizkit3dWidget is not strictly singleton(you can instantiate multiple of them
    //without problems), they all show the same scene because there is only one OsgViz object.
    //since the property viewers are not synchronized, this gets confusing quickly so only
    //allow a single widget.
    if(currentWidget)
    {
        return nullptr;
    }
    currentWidget = new Vizkit3DPluginsWidget(pluginRepo, v3dwidget);
    connect(currentWidget, &Vizkit3DPluginsWidget::deleting,
            this, [this]()
    {
        currentWidget = nullptr;
    });
    return currentWidget;
}

std::string Vizkit3DPlugins::getName()
{
    return "Vizkit3D";
}

unsigned Vizkit3DPlugins::getFlags()
{
    return CanRemoveFields | KeepOpenWithoutFields | PreferSingleWidget |
           WidgetCanHandleMultipleFields | KeepWidgets;
}

std::vector<std::string> Vizkit3DPlugins::getStandaloneSubplugins()
{
    std::vector<std::string> names;
    auto list = pluginRepo->getAllAvailablePlugins();
    for (auto &e : list)
    {
        if (e->typeName.empty())
        {
            names.push_back(e->pluginName);
        }
    }
    return names;
}

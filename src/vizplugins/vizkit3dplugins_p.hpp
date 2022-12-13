
#pragma once

#include "vizkitplugin.hpp"
#include <QMetaMethod>

#ifdef HAVE_TRANSFORMER_TYPEKIT
#include <base/samples/RigidBodyState.hpp>
#include <transformer/BroadcastTypes.hpp>
#include <unordered_map>
#include <QTimer>
#endif

class Vizkit3dPluginRepository;
class Vizkit3dVizHandle;

namespace vizkit3d
{
class Vizkit3DWidget;
}

namespace RTT
{
template<typename T> class InputPort;
}

namespace rock_display {

class Vizkit3DPluginsWidget : public rockdisplay::vizkitplugin::Widget {
    Q_OBJECT;
private:
    vizkit3d::Vizkit3DWidget *v3dwidget;
    Vizkit3dPluginRepository *pluginRepo;
#ifdef HAVE_TRANSFORMER_TYPEKIT
    struct TransformerData {
        RTT::base::OutputPortInterface *broadcaster_port;
        RTT::InputPort<transformer::ConfigurationState> *broadcaster_reader;
        struct PortInfo {
            RTT::base::OutputPortInterface *port;
            RTT::InputPort<base::samples::RigidBodyState> *reader;
        };
        std::unordered_map<transformer::PortTransformationAssociation, PortInfo> ports;
        TransformerData();
    };
    std::unordered_map<orocos_cpp::NameService *, TransformerData> transformerData;
#endif

#if defined(HAVE_TRANSFORMER_TYPEKIT)
    QTimer *rttQueryTimer;

    void setupRttQueryTimer();
#endif
public:
    Vizkit3DPluginsWidget(Vizkit3dPluginRepository *pluginRepo,
        vizkit3d::Vizkit3DWidget *v3dwidget);
    virtual ~Vizkit3DPluginsWidget() override;
    virtual QWidget *getWidget() override;
#ifdef HAVE_TRANSFORMER_TYPEKIT
    void push_rigidbodystate(base::samples::RigidBodyState const *trsf,
                             orocos_cpp::NameService *nameservice);
    void push_transformer_configuration(transformer::ConfigurationState const *state, orocos_cpp::NameService* nameservice);
    void add_transformer_broadcaster_listener(orocos_cpp::NameService* nameservice);
#endif
public slots:
    virtual rockdisplay::vizkitplugin::Field *addOutputPortField(
        const rockdisplay::vizkitplugin::FieldDescription *type,
        std::string const &subpluginname) override;
    virtual rockdisplay::vizkitplugin::Field *addInputPortField(
        const rockdisplay::vizkitplugin::FieldDescription *type,
        std::string const &subpluginname) override;
    virtual rockdisplay::vizkitplugin::Field *addPropertyField(
        const rockdisplay::vizkitplugin::FieldDescription *type,
        std::string const &subpluginname) override;
    virtual rockdisplay::vizkitplugin::StandaloneSubplugin *addStandaloneSubplugin(
        std::string const &subpluginname) override;
    virtual void removeInputPortField(rockdisplay::vizkitplugin::FieldDescription const *field,
                                      rockdisplay::vizkitplugin::Field *f) override;
    virtual void removeOutputPortField(rockdisplay::vizkitplugin::FieldDescription const *field,
                                       rockdisplay::vizkitplugin::Field *f) override;
    virtual void removePropertyField(rockdisplay::vizkitplugin::FieldDescription const *field,
                                     rockdisplay::vizkitplugin::Field *f) override;
    virtual void removeStandaloneSubplugin(rockdisplay::vizkitplugin::StandaloneSubplugin *f)  override;
#if defined(HAVE_TRANSFORMER_TYPEKIT)
private slots:
    void rttQueryTimeout();
#endif
signals:
    void deleting();
};

class Vizkit3dPluginsOutputPortField : public rockdisplay::vizkitplugin::Field {
    Q_OBJECT;
public:
    QObject *plugin;
    QMetaMethod method;

    Vizkit3dPluginsOutputPortField(QObject *plugin, QMetaMethod method);
public slots:
    virtual void updateOutputPort(const rockdisplay::vizkitplugin::ValueHandle *value) override;
};

class Vizkit3dPluginsInputPortField : public rockdisplay::vizkitplugin::Field {
    Q_OBJECT;
public:
    QObject *plugin;
    QMetaMethod method;

    Vizkit3dPluginsInputPortField(QObject *plugin, QMetaMethod method);
public slots:
    virtual void updateInputPort(rockdisplay::vizkitplugin::ValueHandle *value) override;
};

class Vizkit3dPluginsPropertyField : public rockdisplay::vizkitplugin::Field {
    Q_OBJECT;
public:
    QObject *plugin;
    QMetaMethod method;

    Vizkit3dPluginsPropertyField(QObject *plugin, QMetaMethod method);
public slots:
    virtual void updateProperty(rockdisplay::vizkitplugin::ValueHandle *value) override;
};


class Vizkit3dPluginsStandaloneSubplugin : public rockdisplay::vizkitplugin::StandaloneSubplugin {
    Q_OBJECT;
public:
    QObject *plugin;

    Vizkit3dPluginsStandaloneSubplugin(QObject *plugin);
};

#ifdef HAVE_TRANSFORMER_TYPEKIT

class Vizkit3dPluginsTransformerDispatchOutputPortField : public rockdisplay::vizkitplugin::Field {
    Q_OBJECT;
public:
    Vizkit3DPluginsWidget *widget;

    Vizkit3dPluginsTransformerDispatchOutputPortField(Vizkit3DPluginsWidget *widget);
public slots:
    virtual void updateOutputPort(const rockdisplay::vizkitplugin::ValueHandle *value) override;
};

#endif

}

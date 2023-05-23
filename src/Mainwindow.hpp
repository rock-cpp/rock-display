#pragma once

#include <QMainWindow>
#include <QDialog>
#include "NameServiceModel.hpp"
#include "vizplugins/vizkitplugin_p.hpp"
#include "VizHandle.hpp"
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include "TypeConverter.hpp"
#include "TypeConverterFactory.hpp"

QT_BEGIN_NAMESPACE
class QLineEdit;
class QTreeView;
class QTimer;
QT_END_NAMESPACE

namespace Ui
{
class MainWindow;
}

namespace orogen_transports
{
class TypelibMarshallerBase;
}

namespace orocos_cpp {
class OrocosCpp;
}

class TypedItem;
class InputPortItem;

class AddNameServiceDialog : public QDialog
{
    Q_OBJECT
    
public:
    AddNameServiceDialog(QWidget* parent = 0);
    
private:
    QLineEdit *nameServiceIP;
    void addNameService();
    
public slots:
    virtual void accept();
    
signals:
    void requestNameServiceAdd(const std::string &nameServiceIP);
};

class PluginWidgetQMainWindow : public QMainWindow
{
    Q_OBJECT;
protected:
    //override the closeEvent since we want to be able to re-show the window
    virtual void closeEvent(QCloseEvent *ev) override;
signals:
    void closing();
};

class RTTTypelibTypeConverter : public TypeConverter {
private:
    std::string nativeTypename;
    orogen_transports::TypelibMarshallerBase *transport;
    void *transportHandle;//since the Handle is subclass of orogen_transports::TypelibMarshallerBase,
    //we can not forward declare it. so completely remove the type.
public:
    RTTTypelibTypeConverter(orogen_transports::TypelibMarshallerBase *transport,
                            std::string const &nativeTypename);
    virtual ~RTTTypelibTypeConverter() override;
    virtual std::string getResultTypename() const override {
        return nativeTypename;
    }
    orogen_transports::TypelibMarshallerBase *getTransport() const {
        return transport;
    }
    virtual TypeConverter::ConversionResult convertToResult(Typelib::Value const &value,
            const Typelib::Registry *registry) override;
    virtual void refreshFromResult(Typelib::Value &orig_value) override;
};

class RTTTypelibTypeConverterFactory : public TypeConverterFactory {
private:
    std::string nativeTypename;
    orogen_transports::TypelibMarshallerBase *transport;
public:
    RTTTypelibTypeConverterFactory(orogen_transports::TypelibMarshallerBase *transport,
                            std::string const &nativeTypename)
    : nativeTypename(nativeTypename), transport(transport)
    {}
    virtual std::string getResultTypename() const override {
        return nativeTypename;
    }
    orogen_transports::TypelibMarshallerBase *getTransport() const {
        return transport;
    }
    virtual std::unique_ptr<TypeConverter> createConverter() const override;
};

class RTTAliasTypeConverter : public TypeConverter {
private:
    std::string aliasTypename;
public:
    RTTAliasTypeConverter(std::string const &aliasTypename)
    : aliasTypename(aliasTypename)
    {}
    virtual std::string getResultTypename() const override {
        return aliasTypename;
    }
    virtual TypeConverter::ConversionResult convertToResult(Typelib::Value const &value,
            const Typelib::Registry *registry) override;
    virtual void refreshFromResult(Typelib::Value &orig_value) override;
};

class RTTAliasTypeConverterFactory : public TypeConverterFactory {
private:
    std::string aliasTypename;
public:
    RTTAliasTypeConverterFactory(std::string const &aliasTypename)
    : aliasTypename(aliasTypename)
    {}
    virtual std::string getResultTypename() const override {
        return aliasTypename;
    }
    virtual std::unique_ptr<TypeConverter> createConverter() const override;
};

class WidgetInfo {
public:
    rockdisplay::vizkitplugin::Widget *widget;
    rockdisplay::vizkitplugin::Plugin *plugin;
};

class FieldVizHandle : public VizHandle {
    Q_OBJECT;
public:
    rockdisplay::vizkitplugin::Widget *widget;
    rockdisplay::vizkitplugin::Plugin *plugin;
    TypedItem *item;
    rockdisplay::FieldDescriptionImpl *fieldHandle;
    rockdisplay::ValueHandleImpl *valueHandle;
    rockdisplay::vizkitplugin::Field *outputportfield;
    rockdisplay::vizkitplugin::Field *inputportfield;
    rockdisplay::vizkitplugin::Field *propertyfield;
    std::string subpluginname;
    TypeConverterFactory *converterfactory;
    std::unique_ptr<TypeConverter> converter;
    Typelib::Value orig_value;
    bool removing;
    FieldVizHandle() : widget(nullptr), plugin(nullptr), item(nullptr),
        fieldHandle(nullptr), valueHandle(nullptr), outputportfield(nullptr),
        inputportfield(nullptr), propertyfield(nullptr),
        converterfactory(nullptr), removing(false) {}
    ~FieldVizHandle();
signals:
    void fieldRemoved();
public slots:
    virtual void updateVisualizer(Typelib::Value const &value) override;
    /* this sample can be kept around for editing purposes */
    virtual void updateEditable(Typelib::Value const &value) override;
    void plugin_field_destroyed();
private slots:
    void edited(bool force_send);
};

class StandalonePluginHandle {
public:
    WidgetInfo widget;
    std::string subpluginname;
    rockdisplay::vizkitplugin::StandaloneSubplugin *standalonesubplugin;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(orocos_cpp::OrocosCpp &orocos, QWidget *parent = 0);
    ~MainWindow();
    
public slots:
    void prepareMenu(const QPoint &pos);
    void onExpanded(const QModelIndex &index);
    void onCollapsed(const QModelIndex &index);
    void setItemExpanded(const QModelIndex &index, bool expanded=false);
    void addNameService();
    void removeAllPlugins();
    void sortTasks(); //sorts by column 0
    void itemDataEdited(const QModelIndex &index);
    void itemDataEdited(QStandardItem *qitem, bool forceSend = false);

private slots:
    void filterTextEdited(QString const &text);

signals:
    void stopNotifier();
    
private:
    struct AddFieldInfo
    {
        rockdisplay::vizkitplugin::Plugin *plugin;
        std::string subpluginname;
        rockdisplay::vizkitplugin::Widget *widget;
        TypeConverterFactory *converterfactory;
    };

    struct AddWidgetInfo
    {
        rockdisplay::vizkitplugin::Plugin *plugin;
        std::string subpluginname;
        TypeConverterFactory *converterfactory;
    };

    std::vector<rockdisplay::vizkitplugin::Plugin*> plugins;
    std::vector<WidgetInfo> widgets;
    std::unordered_map<rockdisplay::vizkitplugin::Widget*, PluginWidgetQMainWindow *> widgetWindows;//the type may be changed again to allow changing the name of the window and maybe for interaction with the widget(properties?)
    std::unordered_map<rockdisplay::vizkitplugin::Plugin *, std::vector<std::pair<rockdisplay::vizkitplugin::Widget *, PluginWidgetQMainWindow *> > >
    hiddenWidgets;

    std::unordered_multimap<TypedItem *,FieldVizHandle*> fieldVizHandles;
    std::unordered_set<StandalonePluginHandle*> standalonePluginHandles;
    Ui::MainWindow *ui;
    QTimer *uiUpdateTimer;
    QTreeView *view;
    NameServiceModel *model;
    ConfigItemHandlerRepository *handlerrepo;
    AddNameServiceDialog *nameServiceDialog;

    std::map<InputPortItem*,QWidget*> changeconfirms;
    std::unordered_multimap<std::string, TypeConverterFactory*> typeconverters;

    void cleanup();
    rockdisplay::vizkitplugin::Widget *createWidget(rockdisplay::vizkitplugin::Plugin *plugin);
    bool addFieldToWidget(rockdisplay::vizkitplugin::Plugin *plugin,
                                  std::string const &subpluginname,
                                  rockdisplay::vizkitplugin::Widget *widget, TypedItem *ti,
                                  TypeConverterFactory *converterfactory);
    void destroyWidget(rockdisplay::vizkitplugin::Plugin *plugin, rockdisplay::vizkitplugin::Widget *widget);
    void removeFieldFromWidget(FieldVizHandle *fvh, TypedItem *ti);

    void regenerateWidgetsMenu();

    std::vector<FieldVizHandle *> findWidgetsShowingItem(TypedItem *ti);
    std::vector<AddFieldInfo> findWidgetsThatCanShowItemButDontForTheSubplugin(TypedItem *ti);
    std::vector<AddWidgetInfo> findPluginsThatCanCreateWidgetShowingItem(TypedItem *ti);
    std::unordered_map<rockdisplay::vizkitplugin::Widget*, unsigned int> calculateWidgetUsageCount();
    bool populatePluginMenuSection(QMenu *menu, TypedItem *ti);
};

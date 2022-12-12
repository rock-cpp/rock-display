#pragma once

#include <QMainWindow>
#include <QDialog>
#include "NameServiceModel.hpp"
#include <vizkit3d/Vizkit3DWidget.hpp>
#include <rtt/base/DataSourceBase.hpp>

QT_BEGIN_NAMESPACE
class QLineEdit;
class QTreeView;
class QTimer;
QT_END_NAMESPACE

namespace Ui {
    class MainWindow;
}

namespace RTT {
    namespace corba {
        class TaskContextProxy;
    }
}

class TypedItem;
class InputPortItem;
class Vizkit3dPluginRepository;
class PluginHandle;
class VizHandle;


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

class MyVizkit3DWidget : public vizkit3d::Vizkit3DWidget
{
protected:
    //override the closeEvent since we want to be able to re-show the window
    void closeEvent(QCloseEvent *ev);
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    
    Vizkit3dPluginRepository *getPluginRepo()
    {
        return pluginRepo;
    }
    
public slots:
    void prepareMenu(const QPoint &pos);
    void handlePort(QObject *obj);
    void openPlugin(QObject *obj);
    void activateTask();
    void startTask();
    void stopTask();
    void configureTask();
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
    MyVizkit3DWidget widget3d;
    std::vector<PluginHandle*> additionalPlugins;
    Ui::MainWindow *ui;
    QTimer *uiUpdateTimer;
    QTreeView *view;
    NameServiceModel *model;
    RTT::corba::TaskContextProxy *task;
    Vizkit3dPluginRepository *pluginRepo;
    ConfigItemHandlerRepository *handlerrepo;
    AddNameServiceDialog *nameServiceDialog;

    std::map<InputPortItem*,QWidget*> changeconfirms;

    void cleanup();
    std::vector<std::pair<VizHandle *, TypedItem *>> activePlugins;
    void removePlugin(VizHandle *plugin, TypedItem *ti);
    void addPlugin(PluginHandle const *ph, TypedItem *ti);
};

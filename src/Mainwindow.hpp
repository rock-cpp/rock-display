#pragma once

#include <QMainWindow>
#include <QDialog>
#include <QTreeWidget>
#include <QStandardItem>
#include <QStandardItemModel>
#include "TaskModel.hpp"
#include "NameServiceModel.hpp"
#include "Vizkit3dPluginRepository.hpp"
#include <vizkit3d/Vizkit3DWidget.hpp>
#include <thread>

namespace Ui {
    class MainWindow;
}

namespace RTT {
    namespace corba {
        class TaskContextProxy;
    }
}

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
    void handleOutputPort(QObject *obj);
    void openPlugin(QObject *obj);
    void activateTask();
    void startTask();
    void stopTask();
    void configureTask();
    void updateTasks();
    void onExpanded(const QModelIndex &index);
    void onCollapsed(const QModelIndex &index);
    void setItemExpanded(const QModelIndex &index, bool expanded=false);
    void addNameService();
    void removeAllPlugins();
    void sortTasks(); //sorts by column 0
    void updateVisualizer(VizHandle vizhandle, RTT::base::DataSourceBase::shared_ptr data);
    
signals:
    void stopNotifier();
    void stopUIUpdater();
    
private:
    MyVizkit3DWidget widget3d;
    std::vector<PluginHandle> additionalPlugins;
    Ui::MainWindow *ui;
    QTimer *uiUpdateTimer;
    QTreeView *view;
    NameServiceModel *model;
    RTT::corba::TaskContextProxy *task;
    Vizkit3dPluginRepository *pluginRepo;
    AddNameServiceDialog *nameServiceDialog;
    void cleanup();
    std::vector<std::pair<QObject *, TypedItem *>> activePlugins;
    void removePlugin(QObject *plugin, TypedItem *ti);
    void addPlugin(PluginHandle &ph, TypedItem *ti);
};

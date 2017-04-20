#pragma once

#include <QMainWindow>
#include <QTreeWidget>
#include <QStandardItem>
#include <QStandardItemModel>
#include "TaskModel.hpp"
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
    
    void sigIntHandler(int sig)
    {
        cleanup();
    }
    
    void closeEvent(QCloseEvent *event);

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
    
signals:
    void stopNotifier();
    
private:
    vizkit3d::Vizkit3DWidget widget3d;
    std::vector<PluginHandle> additionalPlugins;
    Ui::MainWindow *ui;
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

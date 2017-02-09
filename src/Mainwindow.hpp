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
    
    vizkit3d::Vizkit3DWidget widget3d;
    Vizkit3dPluginRepository *getPluginRepo()
    {
        return pluginRepo;
    }
    
    QThread *notifierThread;

public slots:
    void prepareMenu(const QPoint &pos);
    void handleOutputPort(QObject *obj);
    void openPlugin(QObject *obj);
    void activateTask();
    void startTask();
    void stopTask();
    void configureTask();
    void updateTaskItems();
    void onExpanded(const QModelIndex &index);
    void onCollapsed(const QModelIndex &index);
    void setItemExpanded(const QModelIndex &index, bool expanded=false);
    void addNameService();
    
signals:
    void stopNotifier();
    
private:
    Ui::MainWindow *ui;
    QTreeView *view;
    TaskModel *model;
    RTT::corba::TaskContextProxy *task;
    Vizkit3dPluginRepository *pluginRepo;
    std::thread *modelUpdater;
    AddNameServiceDialog *nameServiceDialog;
};
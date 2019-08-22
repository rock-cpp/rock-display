#pragma once

#include <QStandardItemModel>
#include "TaskItem.hpp"
#include <mutex>

namespace RTT
{
    namespace corba
    {
        class TaskContextProxy;
    }
}

namespace orocos_cpp
{
    class NameService;
}

class Notifier : public QObject
{
    Q_OBJECT
    
    std::map<std::string, RTT::corba::TaskContextProxy *> nameToRegisteredTask;
    std::vector<std::string > disconnectedTasks;
    orocos_cpp::NameService *nameService;
    bool isRunning;
    void queryTasks();
    int connect_trials;
    const int max_connect_trials = 10;
    int numTasks;
  
public:
    explicit Notifier(QObject* parent = 0);
    
    signals:
        void updateTask(RTT::corba::TaskContextProxy* task, const std::string &taskName, bool reconnect);
        void finished();
        void updateNameServiceStatus(const std::string &status);
        void updateTasksStatus(const std::string &status);
        
public slots:
    void stopNotifier();
    void initializeNameService(const std::string &nameServiceIP);
    orocos_cpp::NameService *getNameService()
    {
        return nameService;
    }
    
    void run()
    {
        isRunning = true;
        
        while (isRunning)
        {
            queryTasks();
            usleep(1000);
        }
        
        emit finished();
    }
};

class TaskModel : public QObject
{
    Q_OBJECT
    
    friend class Notifier;
    
    TypedItem nameItem;
    TypedItem statusItem;
    
    QStandardItem tasks;

    std::map<std::string, TaskItem *> nameToItem;
    std::mutex nameToItemMutex;
    
    void updateTaskItem(TaskItem *item);
    
public:
    explicit TaskModel(QObject* parent = 0, const std::string &nameServiceIP = {});
    virtual ~TaskModel();
    void updateTaskItems();
    Notifier *notifier;
    QList<QStandardItem *> getRow();
    QThread *notifierThread;
    
    QStandardItem &getTasks()
    {
        return tasks;
    }
    
    void waitForTerminate();
    
signals:
    void dataChanged(const QModelIndex &i, const QModelIndex &j);
    /**Is emitted every time a new task is added */
    void taskAdded(const TaskItem* taskItem);
    
public slots:
    void onUpdateTask(RTT::corba::TaskContextProxy* task, const std::string &taskName, bool reconnect);
    void updateNameServiceStatus(const std::string &status);
    void updateTasksStatus(const std::string &status);
};


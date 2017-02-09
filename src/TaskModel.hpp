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
    std::vector<orocos_cpp::NameService *> nameServices;
    bool isRunning;
    std::mutex nameServicesMutex;
        
    void queryTasks();
    void queryTasks(orocos_cpp::NameService *nameService);
  
public:
    explicit Notifier(QObject* parent = 0);
    
    signals:
        void updateTask(RTT::corba::TaskContextProxy* task, const std::string &taskName, bool reconnect);
        void finished();
        
public slots:
    void stopNotifier();
    void addNameService(const std::string &nameServiceIP);
    void run()
    {
        isRunning = true;
        
        while (isRunning)
        {
            nameServicesMutex.lock();
            queryTasks();
            nameServicesMutex.unlock();
            usleep(20000);
        }
        
        emit finished();
    }
};

class TaskModel : public QStandardItemModel
{
    Q_OBJECT
    
    friend class Notifier;

    std::map<std::string, TaskItem *> nameToItem;
    std::mutex nameToItemMutex;
    
    void updateTaskItem(TaskItem *item);
    
public:
    explicit TaskModel(QObject* parent = 0);
    virtual ~TaskModel();
    void updateTaskItems();
    Notifier *notifier;
    
public slots:
    void onUpdateTask(RTT::corba::TaskContextProxy* task, const std::string &taskName, bool reconnect);
};
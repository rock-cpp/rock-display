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

class Notifier : public QThread {
    Q_OBJECT
    
    std::map<std::string, RTT::corba::TaskContextProxy *> nameToRegisteredTask;
    std::vector<std::string > disconnectedTasks;
    orocos_cpp::NameService *nameService;
    
    void run()
    {
        while(true)
        {
            queryTasks();
            usleep(10000);
        }
    }
        
    void queryTasks();
  
public:
    explicit Notifier(QObject* parent = 0);
    
    signals:
        void updateTask(RTT::corba::TaskContextProxy* task, const std::string &taskName, bool reconnect);
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
    void updateTaskItems();
    Notifier *notifier;
    
public slots:
    void onUpdateTask(RTT::corba::TaskContextProxy* task, const std::string &taskName, bool reconnect);
};
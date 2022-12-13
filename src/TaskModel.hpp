#pragma once

#include <QStandardItem>
#include <mutex>
#include "TypedItem.hpp"

namespace RTT
{
    namespace corba
    {
        class TaskContextProxy;
    }
}

namespace orocos_cpp
{
    class OrocosCpp;
}

class TaskModelNotifier;
class TaskItem;
class ConfigItemHandlerRepository;

class TaskModel : public QObject
{
    Q_OBJECT
    
    friend class Notifier;
    
    TypedItem nameItem;
    TypedItem statusItem;
    
    QStandardItem tasks;

    std::map<std::string, TaskItem *> nameToItem;
    std::mutex nameToItemMutex;
    ConfigItemHandlerRepository *handlerrepo;
    orocos_cpp::OrocosCpp &orocos;
    TaskModelNotifier *notifier;
    QThread *notifierThread;

    void updateTaskItem(TaskItem *item, bool handleOldData = false);
    
public:
    explicit TaskModel(ConfigItemHandlerRepository *handlerrepo, orocos_cpp::OrocosCpp &orocos, QObject* parent = 0, const std::string &nameServiceIP = {});
    virtual ~TaskModel();
    void updateTaskItems(bool handleOldData = false);
    QList<QStandardItem *> getRow();
    
    QStandardItem &getTasks()
    {
        return tasks;
    }
    
    void waitForTerminate();
    
signals:
    /**Is emitted every time a new task is added */
    void taskAdded(const TaskItem* taskItem);
    
public slots:
    void onUpdateTask(RTT::corba::TaskContextProxy* task, const std::string &taskName, bool reconnect);
    void updateNameServiceStatus(const std::string &status);
    void updateTasksStatus(const std::string &status);
    void stopNotifier();
    void startNotifier();
};


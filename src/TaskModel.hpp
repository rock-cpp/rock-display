#pragma once

#include <QStandardItem>
#include "TypedItem.hpp"

namespace RTT
{
    namespace corba
    {
        class TaskContextProxy;
    }
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
    ConfigItemHandlerRepository *handlerrepo;
    TaskModelNotifier *notifier;
    QThread *notifierThread;
    
    void updateTaskItem(TaskItem *item);
    
public:
    explicit TaskModel(ConfigItemHandlerRepository *handlerrepo, QObject* parent = 0, const std::string &nameServiceIP = {});
    virtual ~TaskModel();
    void updateTaskItems();
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


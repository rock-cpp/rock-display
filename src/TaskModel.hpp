#pragma once

#include <QStandardItemModel>
#include <QStandardItem>

namespace RTT
{
    class TaskContext;
}

namespace orocos_cpp
{
    class NameService;
}

class TaskModel : public QStandardItemModel 
{
    Q_OBJECT
    class TaskData
    {
    public:
        QStandardItem *nameItem;
        QStandardItem *statusItem;
    };
    
    std::map<std::string, TaskData> nameToData;
    orocos_cpp::NameService *nameService;
public:
    explicit TaskModel(QObject* parent = 0);
    void updateTask(RTT::TaskContext *task);
public slots:
    void queryTasks();
};

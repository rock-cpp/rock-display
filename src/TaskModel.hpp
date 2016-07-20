#pragma once

#include <QStandardItemModel>
#include "TaskItem.hpp"

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

    std::map<std::string, TaskItem *> nameToData;
    orocos_cpp::NameService *nameService;
public:
    explicit TaskModel(QObject* parent = 0);
    void updateTask(RTT::TaskContext *task);
public slots:
    void queryTasks();
};

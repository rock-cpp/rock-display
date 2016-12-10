#pragma once

#include <QStandardItemModel>
#include "TaskItem.hpp"

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

class TaskModel : public QStandardItemModel
{
    Q_OBJECT

    std::map<std::string, TaskItem *> nameToData;
    std::map<std::string, TaskItem *> unregisteredTasks;
    orocos_cpp::NameService *nameService;
public:
    explicit TaskModel(QObject* parent = 0);
    void updateTask(RTT::corba::TaskContextProxy *task);
    
public slots:
    void queryTasks();
};

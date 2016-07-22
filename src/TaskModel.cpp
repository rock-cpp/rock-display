#include <rtt/TaskContext.hpp>
#include "TaskModel.hpp"
#include <orocos_cpp/CorbaNameService.hpp>
#include "TaskItem.hpp"

TaskModel::TaskModel(QObject* parent): QStandardItemModel(parent), nameService(new orocos_cpp::CorbaNameService())
{
    setColumnCount(2);
    setHorizontalHeaderLabels(QStringList({"Name","Value"}));
}

void TaskModel::updateTask(RTT::TaskContext* task)
{
    std::string taskName = task->getName();
    auto it = nameToData.find(taskName);
    TaskItem *item = nullptr;
    if(it == nameToData.end())
    {
        item = new TaskItem;
        nameToData.insert(std::make_pair(taskName, item));
        appendRow(item->getRow());
    }
    else
    {
        item = it->second;
    }

    if(!item->update(task))
    {
        //make it gray, disconnect
        //item->setFlags(Qt::NoItemFlags);
        // Reactivate with Qt::ItemIsEnabled
    }
    else
    {
        emit dataChanged(item->updateLeft(), item->updateRight());
    }
}

void TaskModel::queryTasks()
{
    if(!nameService->isConnected())
    {
        if(!nameService->connect())
        {
            std::cout << "Could not connect to Nameserver " << std::endl;
            return;
        }
    }
    std::vector<std::string> tasks = nameService->getRegisteredTasks();

    for(const std::string &tname : tasks)
    {
        RTT::TaskContext *task = nameService->getTaskContext(tname);
        updateTask(task);
    }
}

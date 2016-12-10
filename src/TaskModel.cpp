#include <rtt/TaskContext.hpp>
#include "TaskModel.hpp"
#include <orocos_cpp/CorbaNameService.hpp>
#include "TaskItem.hpp"
#include <rtt/transports/corba/TaskContextProxy.hpp>

TaskModel::TaskModel(QObject* parent): QStandardItemModel(parent), nameService(new orocos_cpp::CorbaNameService())
{
    setColumnCount(2);
    setHorizontalHeaderLabels(QStringList( {"Name","Value"}));
}

void TaskModel::updateTask(RTT::corba::TaskContextProxy* task)
{
    std::cout << "update task " << task->getName() << std::endl;
    std::cout << "number of ports " << task->ports()->getPortNames().size() << std::endl;
    std::string taskName = task->getName();

    auto it = nameToData.find(taskName);
    TaskItem *item = nullptr;

    if(it == nameToData.end())
    {
        std::cout << "add task.." << std::endl;
        item = new TaskItem(task);
        nameToData.insert(std::make_pair(taskName, item));
        appendRow(item->getRow());
    }
    else
    {
        item = it->second;
    }

    if(!item->update())
    {
        //make it gray, disconnect
        //item.setEnabled(false);
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
    std::cout << "get registered tasks.." << std::endl;
    // if there are leftovers getRegisteredTasks() does not return
    // restart omniorb and remove old files solved this..
    std::vector<std::string> tasks;
    try
    {
        tasks = nameService->getRegisteredTasks();
    }
    catch(...)
    {
        std::cout << "could not get regsitered tasks.." << std::endl;
        return;
    }
    std::cout << "got " <<  tasks.size() << " registered tasks" << std::endl;

    for(const std::string &tname : tasks)
    {
        RTT::corba::TaskContextProxy *task = nullptr;
        std::map<std::string, TaskItem*>::iterator taskIt = nameToData.find(tname);
        if (taskIt == nameToData.end() || unregisteredTasks.find(tname) != unregisteredTasks.end())
        {
            std::cout << "get task context: " << tname << std::endl;
            task = RTT::corba::TaskContextProxy::Create(tname);
            std::cout << "get task context for: " << task->getName() << std::endl;
            std::cout << "number of task ports: " << task->ports()->getPorts().size() << std::endl;
        }
        else
        {
            task = taskIt->second->getTaskContext();
        }

        updateTask(task);
    }
}
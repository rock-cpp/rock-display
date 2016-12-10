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

    if (unregisteredTasks.find(taskName) != unregisteredTasks.end())
    {
        item->clearPorts();
        item->updateTaskContext(task);
        unregisteredTasks.erase(taskName);
        std::cout << "cleared all ports.." << std::endl;
    }

    if(item->update())
    {
        emit dataChanged(item->updateLeft(), item->updateRight());
    }
}

void TaskModel::queryTasks()
{
    std::cout << "queryTasks.." << std::endl;
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

    std::map<std::string, TaskItem*>::iterator taskIt = nameToData.begin();
    for (; taskIt != nameToData.end(); taskIt++)
    {
        bool enabled = true;
        if (std::find(tasks.begin(), tasks.end(), taskIt->first) == tasks.end())
        {
            unregisteredTasks[taskIt->first] = taskIt->second;
            enabled = false;
        }

        taskIt->second->getInputPorts().setEnabled(enabled);
        taskIt->second->getOutputPorts().setEnabled(enabled);
        for (QStandardItem *it: taskIt->second->getRow())
        {
            it->setEnabled(enabled);
        }

        for (std::pair<std::string, PortItem *> portItem: taskIt->second->getPorts())
        {
            QList<QStandardItem *>::iterator it = portItem.second->getRow().begin();
            while (it != portItem.second->getRow().end())
            {
                if (*it)
                {
                    (*it)->setEnabled(enabled);
                }
                it++;
            }
        }
    }
}
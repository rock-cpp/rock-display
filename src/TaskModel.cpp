#include <rtt/TaskContext.hpp>
#include "TaskModel.hpp"
#include <orocos_cpp/CorbaNameService.hpp>
#include "TaskItem.hpp"
#include <rtt/transports/corba/TaskContextProxy.hpp>

TaskModel::TaskModel(QObject* parent): QStandardItemModel(parent)
{
    setColumnCount(2);
    setHorizontalHeaderLabels(QStringList( {"Name","Value"}));
    
    notifier = new Notifier(this);
    qRegisterMetaType<std::string>("std::string");
    connect(notifier, SIGNAL(updateTask(RTT::corba::TaskContextProxy*, const std::string &, bool)),
                      SLOT(onUpdateTask(RTT::corba::TaskContextProxy*, const std::string &, bool)));
    connect(notifier, SIGNAL(finished()), this, SLOT(deleteLater()));
}

Notifier::Notifier(QObject* parent)
    : QThread(parent),
      nameService(new orocos_cpp::CorbaNameService()),
      isRunning(false)
{
    
}

void Notifier::stopNotifier()
{
    isRunning = false;
}

void Notifier::queryTasks()
{
    if(!nameService->isConnected())
    {
        if(!nameService->connect())
        {
            std::cout << "could not connect to nameserver.." << std::endl;
            return;
        }
    }
    
    std::vector<std::string> tasks;
    try
    {
        tasks = nameService->getRegisteredTasks();
    }
    catch(...)
    {
        std::cout << "could not get registered tasks.." << std::endl;
        return;
    }

    std::map<std::string, RTT::corba::TaskContextProxy *>::iterator taskIt;
    for(const std::string &tname : tasks)
    {
        RTT::corba::TaskContextProxy *task = nullptr;
        taskIt = nameToRegisteredTask.find(tname);
        std::vector<std::string>::iterator diconnectedTaskIt = std::find(disconnectedTasks.begin(), disconnectedTasks.end(), tname);
        if (taskIt == nameToRegisteredTask.end())
        {
            task = RTT::corba::TaskContextProxy::Create(tname);
            std::cout << "create task context.." << task << std::endl;
            nameToRegisteredTask[tname] = task;
            emit updateTask(task, tname, false);
        }
        else if (diconnectedTaskIt != disconnectedTasks.end())
        {
            disconnectedTasks.erase(diconnectedTaskIt);
            task = RTT::corba::TaskContextProxy::Create(tname);
            nameToRegisteredTask[tname] = task;
            emit updateTask(task, tname, true);
        }
    }

    taskIt = nameToRegisteredTask.begin();
    for (; taskIt != nameToRegisteredTask.end(); taskIt++)
    {
        if (std::find(tasks.begin(), tasks.end(), taskIt->first) == tasks.end() 
            && std::find(disconnectedTasks.begin(), disconnectedTasks.end(), taskIt->first) == disconnectedTasks.end())
        {
            disconnectedTasks.push_back(taskIt->first);
            emit updateTask(nullptr, taskIt->first, false);
        }
    }
}

void TaskModel::onUpdateTask(RTT::corba::TaskContextProxy* task, const std::string &taskName, bool reconnect)
{
    TaskItem *item = nullptr;
    
    nameToItemMutex.lock();
    std::map<std::string, TaskItem*>::iterator itemIt = nameToItem.find(taskName);
    if (itemIt == nameToItem.end())
    {
        item = new TaskItem(task);
        nameToItem.insert(std::make_pair(taskName, item));
        appendRow(item->getRow());
    }
    else
    {     
        item = itemIt->second;
    }
    
    if (!task || reconnect)
    {
        bool enabled = false;
        if (reconnect)
        {
            enabled = true;
        }
        
        item->getInputPorts().setEnabled(enabled);
        item->getOutputPorts().setEnabled(enabled);
        for (QStandardItem *it: item->getRow())
        {
            it->setEnabled(enabled);
        }
    }
    
    if (!task)
    {
        nameToItemMutex.unlock();
        return;
    }
    
    if (reconnect)
    {
        item->refreshPorts = true;
        item->updateTaskContext(task);
    }
    
    updateTaskItem(item);
    nameToItemMutex.unlock();
}

void TaskModel::updateTaskItem(TaskItem *item)
{
    if (item->update())
    {
        emit dataChanged(item->updateLeft(), item->updateRight());
    }
}

void TaskModel::updateTaskItems()
{
    nameToItemMutex.lock();
    for (std::map<std::string, TaskItem *>::iterator itemIter = nameToItem.begin(); itemIter != nameToItem.end(); itemIter++)
    {
        updateTaskItem(itemIter->second);
    }
    nameToItemMutex.unlock();
}

TaskModel::~TaskModel()
{
    for (std::map<std::string, TaskItem *>::iterator itemIter = nameToItem.begin(); itemIter != nameToItem.end(); itemIter++)
    {
        delete itemIter->second;
    }
}
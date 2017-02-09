#include <rtt/TaskContext.hpp>
#include "TaskModel.hpp"
#include <orocos_cpp/CorbaNameService.hpp>
#include "TaskItem.hpp"
#include <rtt/transports/corba/TaskContextProxy.hpp>

TaskModel::TaskModel(QObject* parent, const std::string &nameServiceIP)
    : QObject(parent),
      nameItem(ItemType::NAMESERVICE),
      statusItem(ItemType::NAMESERVICE)
{   
    tasks.setText("Tasks");;
    nameItem.appendRow(&tasks);
    nameItem.setData(this);
    statusItem.setData(this);
    
    if (nameServiceIP.empty())
    {
        nameItem.setText("localhost");
    }
    else
    {
        nameItem.setText(nameServiceIP.c_str());
    }
    
    statusItem.setText("connecting..");
    
    notifier = new Notifier();
    notifier->setNameService(nameServiceIP);
    
    if (notifier->getNameService() && notifier->getNameService()->isConnected())
    {
        statusItem.setText("connected!");
    }
    
    qRegisterMetaType<std::string>("std::string");
    connect(notifier, SIGNAL(updateTask(RTT::corba::TaskContextProxy*, const std::string &, bool)),
                      SLOT(onUpdateTask(RTT::corba::TaskContextProxy*, const std::string &, bool)));
    
    notifierThread = new QThread();
    
    connect(notifierThread, SIGNAL(started()), notifier, SLOT(run()));
    connect(notifier, SIGNAL(finished()), notifierThread, SLOT(quit()));
    connect(notifierThread, SIGNAL(finished()), notifierThread, SLOT(deleteLater()));
    connect(notifier, SIGNAL(updateStatus(const std::string&)), this, SLOT(updateStatus(const std::string &)), Qt::DirectConnection);
    
    notifier->moveToThread(notifierThread);
}

Notifier::Notifier(QObject* parent)
    : QObject(parent),
      isRunning(false),
      connect_trials(0)
{
}

void Notifier::stopNotifier()
{
    std::cout << "stopping notifier thread.." << std::endl;
    isRunning = false;
}

void Notifier::setNameService(const std::string &nameServiceIP)
{
    std::cout << "set nameservice for ip " << nameServiceIP << ".." << std::endl;
    if (nameServiceIP.empty())
    {
        nameService = new orocos_cpp::CorbaNameService();
    }
    else
    {
        nameService = new orocos_cpp::CorbaNameService(nameServiceIP);
    }
}

void Notifier::queryTasks()
{
    if(!nameService->isConnected())
    {
        if(!nameService->connect())
        {
            connect_trials++;
            emit updateStatus(std::string(std::string("connecting (") + std::to_string(connect_trials) + std::string(")")));
            std::cout << "could not connect to nameserver.." << std::endl;
            
            if (connect_trials > max_connect_trials)
            {
                emit updateStatus(std::string(std::string("could not connect in ") + std::to_string(max_connect_trials) + std::string(" trials..")));
                connect_trials = 0;
                isRunning = false;
            }
            return;
        }
        else
        {
            connect_trials = 0;
            emit updateStatus("connected!");
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

NameServiceModel::NameServiceModel(QObject* parent): QStandardItemModel(parent)
{
    setColumnCount(2);
    setHorizontalHeaderLabels(QStringList( {"Name","Value"}));
}

NameServiceModel::~NameServiceModel()
{
    for (TaskModel *taskModel: taskModels)
    {
        delete taskModel;
    }
}

void NameServiceModel::update(const QModelIndex &i, const QModelIndex &j)
{
    emit dataChanged(i, j);
}

void NameServiceModel::addTaskModel(TaskModel* task)
{
    taskModels.push_back(task);
    appendRow(task->getRow());
    connect(task, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)), this, SLOT(update(const QModelIndex &, const QModelIndex &)));
    connect(this, SIGNAL(stopNotifier()), task->notifier, SLOT(stopNotifier()), Qt::DirectConnection);
}

void NameServiceModel::stop()
{
    emit stopNotifier();
}

void NameServiceModel::updateTasks()
{
    for (TaskModel *task: taskModels)
    {
        task->updateTaskItems();
    }
}

void TaskModel::waitForTerminate()
{
    notifierThread->quit();
    if (notifierThread->wait())
    {
        notifierThread->terminate();
        notifierThread->wait();
        std::cout << "notifierThread terminated.." << std::endl;
    }
}

void NameServiceModel::waitForTerminate()
{
    for (TaskModel *task: taskModels)
    {
        task->waitForTerminate();
    }
}

void TaskModel::updateStatus(const std::__cxx11::string& status)
{
    this->statusItem.setText(status.c_str());
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
        tasks.appendRow(item->getRow());
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

void NameServiceModel::addNameService(const std::__cxx11::string& nameServiceIP)
{
    if (nameServiceIP.empty())
    {
        return;
    }
    
    TaskModel *newModel = new TaskModel(this, nameServiceIP);
    addTaskModel(newModel);
    newModel->notifierThread->start();
}

void TaskModel::updateTaskItem(TaskItem *item)
{
    if (item->update())
    {
        emit dataChanged(item->updateLeft(), item->updateRight());
    }
}

QList< QStandardItem* > TaskModel::getRow()
{
    return {&nameItem, &statusItem};
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
    
    delete notifier;
}
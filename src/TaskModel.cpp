#include <rtt/TaskContext.hpp>
#include "TaskModel.hpp"
#include <orocos_cpp/CorbaNameService.hpp>
#include "TaskItem.hpp"
#include <rtt/transports/corba/TaskContextProxy.hpp>
#include <boost/thread.hpp>
#include <base-logging/Logging.hpp>
#include <omniORB4/CORBA.h>

Notifier::Notifier(QObject* parent)
    : QObject(parent),
      isRunning(false),
      connect_trials(0),
      numTasks(0)
{
}

void Notifier::stopNotifier()
{
    isRunning = false;
}

void Notifier::initializeNameService(const std::string &nameServiceIP)
{
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
    // check if connection to nameservice is established
    if(!nameService->isConnected())
    {
        // connect to nameservice
        if(!nameService->connect())
        {
            connect_trials++;
            emit updateNameServiceStatus(std::string(std::string("connecting (") + std::to_string(connect_trials) + std::string(")")));
            
            if (connect_trials > max_connect_trials)
            {
                emit updateNameServiceStatus(std::string(std::string("could not connect in ") + std::to_string(max_connect_trials) + std::string(" trials..")));
                connect_trials = 0;
                isRunning = false;
            }
            return;
        }
        else
        {
            connect_trials = 0;
            emit updateNameServiceStatus("connected!");
        }
    }
    
    std::vector<std::string> tasks;
    try
    {
        tasks = nameService->getRegisteredTasks();
        int numTasksCurrent = tasks.size();
        
        if (numTasksCurrent != numTasks)
        {
            numTasks = numTasksCurrent;
            emit updateTasksStatus(std::string(std::string("Tasks [") + std::to_string(numTasks) + std::string("]")));
        }
    }
    catch (CosNaming::NamingContext::NotFound& ex)
    {
        emit updateNameServiceStatus(std::string("CORBA: failed to get registered tasks.."));
    }
    catch(...)
    {
        emit updateNameServiceStatus(std::string("connected: could not get registered tasks.."));
        return;
    }

    std::map<std::string, RTT::corba::TaskContextProxy *>::iterator taskIt;
    for(const std::string &tname : tasks)
    {
        RTT::corba::TaskContextProxy *task = nullptr;
        taskIt = nameToRegisteredTask.find(tname);
        std::vector<std::string>::iterator disconnectedTaskIt = std::find(disconnectedTasks.begin(), disconnectedTasks.end(), tname);
        bool wasDisconnected = (disconnectedTaskIt != disconnectedTasks.end());
        // connect/reconnect of task
        if (taskIt == nameToRegisteredTask.end() || wasDisconnected)
        {
            if (wasDisconnected)
            {
                disconnectedTasks.erase(disconnectedTaskIt);
            }
            
            task = RTT::corba::TaskContextProxy::Create(tname);
            if (!task)
            {
                continue;
            }
            
            const RTT::DataFlowInterface *dfi = task->ports();
            if (!dfi || dfi->getPorts().size() == 0)
            {
                continue;
            }
            
            nameToRegisteredTask[tname] = task;
            emit updateNameServiceStatus(std::string((wasDisconnected ? std::string("reconnect") : std::string("connect")) + std::string(" of task ") + tname + std::string("..")));
            emit updateTask(task, tname, wasDisconnected);
        }
    }

    taskIt = nameToRegisteredTask.begin();
    for (; taskIt != nameToRegisteredTask.end(); taskIt++)
    {
        // check if any of the registered tasks have disconnected
        if (std::find(tasks.begin(), tasks.end(), taskIt->first) == tasks.end() 
            && std::find(disconnectedTasks.begin(), disconnectedTasks.end(), taskIt->first) == disconnectedTasks.end())
        {
            disconnectedTasks.push_back(taskIt->first);
            emit updateNameServiceStatus(std::string(std::string("connected: task ") + taskIt->first + std::string(" disconnected..")));
            emit updateTask(nullptr, taskIt->first, false);
        }
    }
}

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
    notifier->initializeNameService(nameServiceIP);
    
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
    connect(notifier, SIGNAL(updateNameServiceStatus(const std::string&)), this, SLOT(updateNameServiceStatus(const std::string &)), Qt::DirectConnection);
    connect(notifier, SIGNAL(updateTasksStatus(const std::string&)), this, SLOT(updateTasksStatus(const std::string &)), Qt::DirectConnection);
    
    notifier->moveToThread(notifierThread);
}

TaskModel::~TaskModel()
{
    for (std::map<std::string, TaskItem *>::iterator itemIter = nameToItem.begin(); itemIter != nameToItem.end(); itemIter++)
    {
        delete itemIter->second;
    }
    
    delete notifier;
}

void TaskModel::waitForTerminate()
{
    notifierThread->quit();
    if (notifierThread->wait(100))
    {
        notifierThread->terminate();
    }
}

void TaskModel::updateNameServiceStatus(const std::string& status)
{
    this->statusItem.setText(status.c_str());
}

void TaskModel::updateTasksStatus(const std::string& status)
{
    this->tasks.setText(status.c_str());
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
        emit taskAdded(item);
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
        // reset task item and its output ports on disconnect
        item->reset();
        nameToItemMutex.unlock();
        return;
    }
    
    if (reconnect)
    {
        item->setRefreshPorts();
        item->updateTaskContext(task);
    }
    
    updateTaskItem(item);
    nameToItemMutex.unlock();
}

void TaskModel::updateTaskItem(TaskItem *item)
{
    try
    {
        if (item->update())
        {
            emit dataChanged(item->updateLeft(), item->updateRight());
        }
    }
    catch (const CORBA::TRANSIENT& ex)
    {
        item->reset();
        LOG_WARN_S << "caught CORBA::TRANSIENT exception..";
    }
    catch (const CORBA::COMM_FAILURE& ex)
    {
        item->reset();
        LOG_WARN_S << "caught CORBA::COMM_FAILURE exception..";
    }
    catch (const CORBA::OBJ_ADAPTER& ex)
    {
        item->reset();
        LOG_WARN_S << "caught CORBA::OBJ_ADAPTER exception..";
    }
    catch (const CORBA::Exception& ex)
    {
        item->reset();
        LOG_WARN_S << "caught CORBA::EXCEPTION exception..";
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
    connect(task, SIGNAL(taskAdded(const TaskItem*)), this, SLOT(taskAdded(const TaskItem*)));
}

void NameServiceModel::taskAdded(const TaskItem*)
{
    emit rowAdded();
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

void NameServiceModel::waitForTerminate()
{
    for (TaskModel *task: taskModels)
    {
        task->waitForTerminate();
    }
}

void NameServiceModel::addNameService(const std::string& nameServiceIP)
{
    if (nameServiceIP.empty())
    {
        return;
    }
    
    TaskModel *newModel = new TaskModel(this, nameServiceIP);
    addTaskModel(newModel);
    newModel->notifierThread->start();
}
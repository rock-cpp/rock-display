#include <rtt/TaskContext.hpp>
#include "TaskModel.hpp"
#include <orocos_cpp/CorbaNameService.hpp>
#include "TaskItem.hpp"
#include <rtt/transports/corba/TaskContextProxy.hpp>
#include <boost/thread.hpp>
#include <base-logging/Logging.hpp>
#include <omniORB4/CORBA.h>
#include "TaskModelNotifier.hpp"

TaskModel::TaskModel(ConfigItemHandlerRepository *handlerrepo, QObject* parent, const std::string &nameServiceIP)
    : QObject(parent),
      nameItem(ItemType::NAMESERVICE),
      statusItem(ItemType::NAMESERVICE),
      handlerrepo(handlerrepo)
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
    
    notifier = new TaskModelNotifier();
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
    waitForTerminate();
    for (std::map<std::string, TaskItem *>::iterator itemIter = nameToItem.begin(); itemIter != nameToItem.end(); itemIter++)
    {
        delete itemIter->second;
    }
    
    delete notifier;
}

void TaskModel::waitForTerminate()
{
    //this runs inside a helper thread
    notifierThread->quit();
    if (notifierThread->wait(500))
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
    //this runs inside the GUI thread
    assert(qApp->thread() == QThread::currentThread());
    TaskItem *item = nullptr;
    
    {
        std::lock_guard<std::mutex> g(nameToItemMutex);
        std::map<std::string, TaskItem*>::iterator itemIt = nameToItem.find(taskName);
        if (itemIt == nameToItem.end())
        {
            item = new TaskItem(task, handlerrepo);
            nameToItem.insert(std::make_pair(taskName, item));
            tasks.appendRow(item->getRow());
            emit taskAdded(item);
        }
        else
        {
            item = itemIt->second;
        }
    }
    //the lock can be dropped because TaskItems will never be deconstructed while
    //(non-GUI) threads still run.

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
        return;
    }
    
    if (reconnect)
    {
        item->setRefreshPorts();
        item->updateTaskContext(task);
    }
    
    updateTaskItem(item, true);
}

void TaskModel::updateTaskItem(TaskItem *item, bool updateUI, bool handleOldData)
{
    try
    {
        item->update(updateUI, handleOldData);
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

void TaskModel::updateTaskItems(bool updateUI, bool handleOldData)
{
    assert(qApp->thread() == QThread::currentThread() || !updateUI);
    std::vector<TaskItem *>items;
    {
        std::lock_guard<std::mutex> g(nameToItemMutex);
        for (auto itemIter = nameToItem.begin(); itemIter != nameToItem.end(); itemIter++)
        {
            items.push_back(itemIter->second);
        }
    }
    //the lock can be dropped because TaskItems will never be deconstructed while
    //(non-GUI) threads still run.
    //this has been extracted to keep the potential of concurrent access in the contained
    //items. that kind of concurrent access is always possible, but much rarer when only
    //initiated by user action. if the copy above turns out to be a performance issue,
    //just move the updateTaskItem back under the lock.
    for(auto &item : items)
    {
        updateTaskItem(item, updateUI, handleOldData);
    }
}

void TaskModel::stopNotifier()
{
    notifier->stopNotifier();
}

void TaskModel::startNotifier()
{
    notifierThread->start();
}

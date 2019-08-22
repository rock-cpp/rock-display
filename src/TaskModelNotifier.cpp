#include <orocos_cpp/CorbaNameService.hpp>
#include <rtt/transports/corba/TaskContextProxy.hpp>
#include "TaskModelNotifier.hpp"

TaskModelNotifier::TaskModelNotifier(QObject* parent)
        : QObject(parent),
          isRunning(false),
          connect_trials(0),
          numTasks(0)
{
}

void TaskModelNotifier::stopNotifier()
{
    isRunning = false;
}

void TaskModelNotifier::initializeNameService(const std::string &nameServiceIP)
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

void TaskModelNotifier::queryTasks()
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
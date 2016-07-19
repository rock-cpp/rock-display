#include <rtt/TaskContext.hpp> 
#include "TaskModel.hpp"
#include <orocos_cpp/CorbaNameService.hpp>

TaskModel::TaskModel(QObject* parent): QStandardItemModel(parent), nameService(new orocos_cpp::CorbaNameService())
{
    setColumnCount(2);
    setHorizontalHeaderLabels(QStringList({"Name","Value"}));
}


void TaskModel::updateTask(RTT::TaskContext* task)
{
    std::string taskName = task->getName();
    auto it = nameToData.find(taskName);
    TaskData data;
    if(it == nameToData.end())
    {
        data.nameItem = new QStandardItem(taskName.c_str());
        data.statusItem = new QStandardItem();
        nameToData.insert(std::make_pair(taskName, data));
        
        appendRow({data.nameItem, data.statusItem});
    }
    else
    {
        data = it->second;
    }
    
    QString stateString;
    RTT::base::TaskCore::TaskState state = task->getTaskState();
    switch(state)
    {
        case RTT::base::TaskCore::Exception:
            stateString = "Exception";
            break;
        case RTT::base::TaskCore::FatalError:
            stateString = "FatalError";
            break;
        case RTT::base::TaskCore::Init:
            stateString = "Init";
            break;
        case RTT::base::TaskCore::PreOperational:
            stateString = "PreOperational";
            break;
        case RTT::base::TaskCore::Running:
            stateString = "Running";
            break;
        case RTT::base::TaskCore::RunTimeError:
            stateString = "RunTimeError";
            break;
        case RTT::base::TaskCore::Stopped:
            stateString = "Stopped";
            break;
    }
    data.statusItem->setText(stateString);

    emit dataChanged(data.statusItem->index(), data.statusItem->index());
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

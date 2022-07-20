#include "NameServiceModel.hpp"
#include "TaskModel.hpp"
#include "TaskModelNotifier.hpp"
#include <QThread>

NameServiceModel::NameServiceModel(ConfigItemHandlerRepository *handlerrepo, QObject* parent)
    : QStandardItemModel(parent), handlerrepo(handlerrepo)
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

void NameServiceModel::addTaskModel(TaskModel* task)
{
    taskModels.push_back(task);
    appendRow(task->getRow());
    connect(this, SIGNAL(stopNotifier()), task, SLOT(stopNotifier()), Qt::DirectConnection);
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

    TaskModel *newModel = new TaskModel(handlerrepo, this, nameServiceIP);
    addTaskModel(newModel);
    newModel->startNotifier();
}

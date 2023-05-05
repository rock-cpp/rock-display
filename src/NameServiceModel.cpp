#include "NameServiceModel.hpp"
#include "TaskModel.hpp"
#include "TaskModelNotifier.hpp"
#include <QThread>
#ifndef NDEBUG
#include <QApplication>
#endif
#include <cassert>

NameServiceModel::NameServiceModel(ConfigItemHandlerRepository *handlerrepo, orocos_cpp::OrocosCpp &orocos, QObject* parent)
    : QStandardItemModel(parent), handlerrepo(handlerrepo), orocos(orocos)
{
    setColumnCount(2);
    setHorizontalHeaderLabels(QStringList( {"Name","Value"}));
}

NameServiceModel::~NameServiceModel()
{
    //locking not required, no threads left during deconstruction
    for (TaskModel *taskModel: taskModels)
    {
        delete taskModel;
    }
}

void NameServiceModel::addTaskModel(TaskModel* task)
{
    //this runs inside the GUI thread
    assert(qApp->thread() == QThread::currentThread());
    taskModels.push_back(task);
    appendRow(task->getRow());
    connect(this, SIGNAL(stopNotifier()), task, SLOT(stopNotifier()), Qt::DirectConnection);
    connect(task, SIGNAL(taskAdded(const TaskItem*)), this, SLOT(taskAdded(const TaskItem*)));
}

void NameServiceModel::taskAdded(const TaskItem*)
{
    //this runs inside the GUI thread
    assert(qApp->thread() == QThread::currentThread());
    emit rowAdded();
}


void NameServiceModel::stop()
{
    emit stopNotifier();
}

void NameServiceModel::updateTasks(bool handleOldData)
{
    for (TaskModel *task: taskModels)
    {
        task->updateTaskItems(handleOldData);
    }
}

void NameServiceModel::waitForTerminate()
{
    //this runs inside a worker thread
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

    TaskModel *newModel = new TaskModel(handlerrepo, orocos, this, nameServiceIP);
    addTaskModel(newModel);
    newModel->startNotifier();
}

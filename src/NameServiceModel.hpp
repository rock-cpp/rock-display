#pragma once
#include <QStandardItemModel>
#include <vector>

class TaskModel;
class TaskItem;
class ConfigItemHandlerRepository;

namespace orocos_cpp {
class OrocosCpp;
}

class NameServiceModel : public QStandardItemModel
{
    Q_OBJECT

    std::vector<TaskModel *> taskModels;
    ConfigItemHandlerRepository *handlerrepo;
    orocos_cpp::OrocosCpp &orocos;

public:
    explicit NameServiceModel(ConfigItemHandlerRepository *handlerrepo, orocos_cpp::OrocosCpp &orocos, QObject* parent = 0);
    virtual ~NameServiceModel();
    void addTaskModel(TaskModel *task);

    void updateTasks(bool handleOldData = false);
    void waitForTerminate();

    void notifyItemDataEdited(const QModelIndex &i) const
    {
        emit itemDataEdited(i);
    }

public slots:
    void stop();
    void addNameService(const std::string &nameServiceIP);
    void taskAdded(const TaskItem* task);

signals:
    void stopNotifier();
    /**emitted every time a new task is added to one of the task models */
    void rowAdded();
    void itemDataEdited(const QModelIndex &i) const;
};

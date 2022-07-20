#pragma once
#include <QStandardItemModel>
#include <vector>

class TaskModel;
class TaskItem;
class ConfigItemHandlerRepository;

class NameServiceModel : public QStandardItemModel
{
    Q_OBJECT

    std::vector<TaskModel *> taskModels;
    ConfigItemHandlerRepository *handlerrepo;

public:
    explicit NameServiceModel(ConfigItemHandlerRepository *handlerrepo, QObject* parent = 0);
    virtual ~NameServiceModel();
    void addTaskModel(TaskModel *task);

    void updateTasks(bool updateUI = true, bool handleOldData = false);
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

#pragma once
#include <QStandardItemModel>

class TaskModel;
class TaskItem;

class NameServiceModel : public QStandardItemModel
{
    Q_OBJECT

    std::vector<TaskModel *> taskModels;

public:
    explicit NameServiceModel(QObject* parent = 0);
    virtual ~NameServiceModel();
    void addTaskModel(TaskModel *task);

    void updateTasks();
    void waitForTerminate();

public slots:
            void update(const QModelIndex &i, const QModelIndex &j);
    void stop();
    void addNameService(const std::string &nameServiceIP);
    void taskAdded(const TaskItem* task);

    signals:
            void stopNotifier();
    /**emitted every time a new task is added to one of the task models */
    void rowAdded();
};
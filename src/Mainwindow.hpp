#pragma once

#include <QMainWindow>
#include <QTreeWidget>
#include <QStandardItem>
#include <QStandardItemModel>
#include "TaskModel.hpp"

namespace Ui {
    class MainWindow;
}

namespace RTT {
    class TaskContext;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void prepareMenu(const QPoint & pos);
    void queryTasks();
    void activateTask();
    void startTask();
    void stopTask();
    void configureTask();

private:
    Ui::MainWindow *ui;
    QTreeView *view;
    TaskModel *model;
    RTT::TaskContext *task;
};

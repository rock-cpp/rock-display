#pragma once

#include <QMainWindow>
#include <QTreeWidget>
#include <QStandardItem>
#include <QStandardItemModel>
#include "TaskModel.hpp"

namespace Ui {
    class MainWindow;
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

private:
    Ui::MainWindow *ui;
    QTreeView *view;
    TaskModel *model;
};

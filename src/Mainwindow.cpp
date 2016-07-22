#include "Mainwindow.hpp"
#include "ui_task_inspector_window.h"
#include "Types.hpp"
#include "TypedItem.hpp"
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    view = ui->treeView;
    view->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(view, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(prepareMenu(QPoint)));

    model = new TaskModel(this);

    view->setModel(model);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::prepareMenu(const QPoint & pos)
{
    QModelIndex mi = view->indexAt(pos);
    QStandardItem *item = model->itemFromIndex(mi);
    QMenu menu(this);

    if (TypedItem *ti = dynamic_cast<TypedItem*>(item)) {
        qDebug() << ti->text();

        switch (ti->type()) {
            case ItemType::TASK:
                {
                QAction *act = menu.addAction("Activate");
                QAction *sta = menu.addAction("Start");
                QAction *sto = menu.addAction("Stop");
                QAction *con = menu.addAction("Configure");
                /*
                QSignalMapper* signalMapper = new QSignalMapper (this);
                connect(act, SIGNAL(triggered()), signalMapper, SLOT(map()));
                connect(sta, SIGNAL(triggered()), signalMapper, SLOT(map()));
                connect(sto, SIGNAL(triggered()), signalMapper, SLOT(map()));
                connect(con, SIGNAL(triggered()), signalMapper, SLOT(map()));

                signalMapper->setMapping(act, new SignalInformation(SignalInformation::CmdType::ACTIVATE, item->text()));
                signalMapper->setMapping(sta, new SignalInformation(SignalInformation::CmdType::START, item->text()));
                signalMapper->setMapping(sto, new SignalInformation(SignalInformation::CmdType::STOP, item->text()));
                signalMapper->setMapping(con, new SignalInformation(SignalInformation::CmdType::CONFIGURE, item->text())); */

                //connect(signalMapper, SIGNAL(mapped(QObject *)), this, SLOT(executeCmd(QObject *)));
                }
                break;
            case ItemType::PORT:
                // for (b : bla) {}
                menu.addAction("Widget");
                break;
            default:
                printf("Falscher Typ %d\n", ti->type());
        }

        QPoint pt(pos);
        menu.exec(view->mapToGlobal(pos));
    } else {
        printf("Cast kaputt... Type: %d\n", item->type()); //TODO remove after testing
    }
}

void MainWindow::queryTasks() {
    model->queryTasks();
}

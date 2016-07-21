#include "RDItemDelegate.hpp"
#include <QAction>
#include <QMenu>
#include <QMouseEvent>
#include <QSignalMapper>

#include "Types.hpp"

#include <stdio.h> //remove after testing

RDItemDelegate::RDItemDelegate(QTreeView* treeView) : QItemDelegate(), _treeView(treeView)
{

}

void RDItemDelegate::showContextMenu(QStandardItem* item, const QPoint& globalPos)
{
    QMenu menu;

    switch (item->type()) {
        case ItemType::TASK:
            {
            QAction *act = menu.addAction("Activate");
            QAction *sta = menu.addAction("Start");
            QAction *sto = menu.addAction("Stop");
            QAction *con = menu.addAction("Configure");

            QSignalMapper* signalMapper = new QSignalMapper (this);
            connect(act, SIGNAL(triggered()), signalMapper, SLOT(map()));
            connect(sta, SIGNAL(triggered()), signalMapper, SLOT(map()));
            connect(sto, SIGNAL(triggered()), signalMapper, SLOT(map()));
            connect(con, SIGNAL(triggered()), signalMapper, SLOT(map()));

            signalMapper->setMapping(act, new SignalInformation(SignalInformation::CmdType::ACTIVATE, item->text()));
            signalMapper->setMapping(sta, new SignalInformation(SignalInformation::CmdType::START, item->text()));
            signalMapper->setMapping(sto, new SignalInformation(SignalInformation::CmdType::STOP, item->text()));
            signalMapper->setMapping(con, new SignalInformation(SignalInformation::CmdType::CONFIGURE, item->text()));

            connect(signalMapper, SIGNAL(mapped(QObject *)), this, SLOT(executeCmd(QObject *)));
            }
            break;
        case ItemType::PORT:
            // for (b : bla) {}
            menu.addAction("Widget");
            break;
        default:
            printf("Falscher Typ %d\n", item->type());
    }

    menu.exec(globalPos);
}

bool RDItemDelegate::editorEvent(QEvent* event, TaskModel* model,
    const QStyleOptionViewItem& option, const QModelIndex& index)
{
    QMouseEvent* mouseEvent = NULL;

    if (event->type() == QEvent::MouseButtonPress)
    {
        mouseEvent = static_cast<QMouseEvent*>(event);
    }

    if (mouseEvent && mouseEvent->button() == Qt::RightButton)
    {
        QStandardItem* item = model->itemFromIndex(index.model()->index(0,0));

        printf("Type: %d\n", item->type());

        showContextMenu(item, mouseEvent->globalPos());
        return true;
    }

    return QAbstractItemDelegate::editorEvent(event, model, option, index);
}

void RDItemDelegate::executeCmd(QObject *obj) 
{
    SignalInformation *si = static_cast<SignalInformation*>(obj);

    printf("CMD: %d\n", si->cmd());
}

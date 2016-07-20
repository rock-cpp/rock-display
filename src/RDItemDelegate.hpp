#pragma once

#include <QItemDelegate>
#include <QStandardItem>
#include <QTreeView>
#include <QMouseEvent>
#include <QAction>
#include "TaskModel.hpp"
#include "Types.hpp"

class RDItemDelegate : public QItemDelegate
{
public:
    RDItemDelegate(QTreeView* treeView) : _treeView(treeView)
    {
        // Do nothing.
    }

    virtual bool editorEvent(QEvent* event, TaskModel* model,
        const QStyleOptionViewItem& option, const QModelIndex& index)
    {
        QMouseEvent* mouseEvent = NULL;

        if (event->type() == QEvent::MouseButtonPress)
        {
            mouseEvent = static_cast<QMouseEvent*>(event);
        }

        if (mouseEvent and mouseEvent->button() == Qt::RightButton)
        {
            QStandardItem* item = model->itemFromIndex(index.model()->index(0,0));
            showContextMenu(item, mouseEvent->globalPos());
            return true;
        }

        return QAbstractItemDelegate::editorEvent(event, model, option, index);
    }

private:
    void showContextMenu(QStandardItem* item, const QPoint& globalPos)
    {
        QMenu menu;

        switch (item->type()) {
            case ItemType::TASK:
                {
                QAction *act = menu.addAction("Activate");
                QAction *sta = menu.addAction("Start");
                QAction *sto = menu.addAction("Stop");
                QAction *con = menu.addAction("Configure");

                //connect(act, SIGNAL(triggered()), this, SLOT(activate()));
                //connect(sta, SIGNAL(triggered()), this, SLOT(start()));
                //connect(sto, SIGNAL(triggered()), this, SLOT(stop()));
                //connect(con, SIGNAL(triggered()), this, SLOT(configure()));
                }
                break;

            case ItemType::PORT:
                // for (b : bla) {}
                menu.addAction("Widget");
                break;
        }

        menu.exec(globalPos);
    }

private:
    QTreeView* _treeView;
};

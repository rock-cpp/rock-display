#pragma once

#include <QTreeView>
#include <QItemDelegate>
#include <QStandardItem>
#include <QPoint>
#include <QModelIndex>
#include <QEvent>
#include <QObject>

#include "TaskModel.hpp"
#include "Types.hpp"

class RDItemDelegate : public QItemDelegate
{
    Q_OBJECT
    
public:
    RDItemDelegate(QTreeView* treeView);

    virtual bool editorEvent(QEvent* event, TaskModel* model,
        const QStyleOptionViewItem& option, const QModelIndex& index);

public slots:
    void executeCmd(QObject *obj);

private:
    void showContextMenu(QStandardItem* item, const QPoint& globalPos);
    QTreeView* _treeView;
};

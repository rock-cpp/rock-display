
#include "NameServiceItemDelegate.hpp"

NameServiceItemDelegate::NameServiceItemDelegate(QObject* parent)
    : QStyledItemDelegate(parent)
{

}

NameServiceItemDelegate::~NameServiceItemDelegate()
{
}

void NameServiceItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const 
{
    QStyledItemDelegate::paint(painter, option, index);
}

QWidget *NameServiceItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    return QStyledItemDelegate::createEditor(parent, option, index);
}

void NameServiceItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QStyledItemDelegate::setEditorData(editor, index);
}

void NameServiceItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QStyledItemDelegate::setModelData(editor, model, index);
}

void NameServiceItemDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyledItemDelegate::updateEditorGeometry(editor, option, index);
}

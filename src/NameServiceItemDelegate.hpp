
#pragma once
#include <QStyledItemDelegate>
#include <unordered_map>

class NameServiceItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT

private:
    void defaultPaint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
public:
    explicit NameServiceItemDelegate(QObject* parent = 0);
    virtual ~NameServiceItemDelegate() override;
    
    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    
    virtual QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    virtual void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    virtual void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
    virtual void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

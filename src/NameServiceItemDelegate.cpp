
#include "NameServiceItemDelegate.hpp"
#include "NameServiceModel.hpp"
#include <QMetaProperty>
#include "TypedItem.hpp"
#include "ConfigItem.hpp"
#include "PortItem.hpp"
#include "Types.hpp"
#include "ConfigItemHandler.hpp"
#include <QApplication>

NameServiceItemDelegate::NameServiceItemDelegate(QObject* parent)
    : QStyledItemDelegate(parent)
{

}

NameServiceItemDelegate::~NameServiceItemDelegate()
{
}

static QColor mixRgb(QColor const &a, QColor const &b, float ratio)
{
    QColor res;
    res.setAlphaF(a.alphaF()*ratio+b.alphaF()*(1.0-ratio));
    res.setRedF  (a.redF()  *ratio+b.redF()  *(1.0-ratio));
    res.setGreenF(a.greenF()*ratio+b.greenF()*(1.0-ratio));
    res.setBlueF (a.blueF() *ratio+b.blueF() *(1.0-ratio));
    return res;
}

void NameServiceItemDelegate::defaultPaint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    bool modified = index.model()->data(index, ItemBase::ModifiedRole).toBool();
    if (!modified)
    {
        QStyledItemDelegate::paint(painter, option, index);
        return;
    }

    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);
    opt.palette.setBrush(QPalette::Text, mixRgb(opt.palette.color(QPalette::Text), Qt::red, 0.3));
    opt.palette.setBrush(QPalette::HighlightedText,  mixRgb(opt.palette.color(QPalette::HighlightedText), Qt::red, 0.3));
    QApplication::style()->drawControl(QStyle::CE_ItemViewItem, &opt, painter, nullptr);
}

void NameServiceItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const 
{
    NameServiceModel const *nsmodel = static_cast<NameServiceModel const *>(index.model());

    QStandardItem *item = nsmodel->itemFromIndex(index);

    TypedItem *ti = dynamic_cast<TypedItem*>(item);
    if (!ti)
    {
        defaultPaint(painter, option, index);
        return;
    }

    ItemBase* itembase = nullptr;
    if (ti->type() == ItemType::INPUTPORT)
        itembase = static_cast<PortItem *>(ti->getData())->getItemBase().get();
    else if (ti->type() == ItemType::EDITABLEITEM)
        itembase = static_cast<ItemBase *>(ti->getData());
    else
    {
        defaultPaint(painter, option, index);
        return;
    }

    for (auto &h : itembase->getHandlerStack())
    {
        if (h->flags() & ConfigItemHandler::Flags::CustomPaint)
        {
            h->paint(painter, option, index);
            return;
        }
    }

    defaultPaint(painter, option, index);
}

QWidget *NameServiceItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    NameServiceModel const *nsmodel = static_cast<NameServiceModel const *>(index.model());

    QStandardItem *item = nsmodel->itemFromIndex(index);

    TypedItem *ti = dynamic_cast<TypedItem*>(item);
    if (!ti)
        return QStyledItemDelegate::createEditor(parent, option, index);

    ItemBase* itembase = nullptr;
    if (ti->type() == ItemType::INPUTPORT)
        itembase = static_cast<PortItem *>(ti->getData())->getItemBase().get();
    else if (ti->type() == ItemType::EDITABLEITEM)
        itembase = static_cast<ItemBase *>(ti->getData());
    else
        return QStyledItemDelegate::createEditor(parent, option, index);

    for (auto &h : itembase->getHandlerStack())
    {
        if (h->flags() & ConfigItemHandler::Flags::CustomEditor)
        {
            return h->createEditor(this, parent, option, index);
        }
    }

    return QStyledItemDelegate::createEditor(parent, option, index);
}

void NameServiceItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    NameServiceModel const *nsmodel = static_cast<NameServiceModel const *>(index.model());

    QStandardItem *item = nsmodel->itemFromIndex(index);

    TypedItem *ti = dynamic_cast<TypedItem*>(item);
    if (!ti)
    {
        QStyledItemDelegate::setEditorData(editor, index);
        return;
    }

    ItemBase* itembase = nullptr;
    if (ti->type() == ItemType::INPUTPORT)
        itembase = static_cast<PortItem *>(ti->getData())->getItemBase().get();
    else if (ti->type() == ItemType::EDITABLEITEM)
        itembase = static_cast<ItemBase *>(ti->getData());
    else
    {
        QStyledItemDelegate::setEditorData(editor, index);
        return;
    }

    for (auto &h : itembase->getHandlerStack())
    {
        if (h->flags() & ConfigItemHandler::Flags::CustomEditor)
        {
            h->setEditorData(editor, index);
            return;
        }
    }

    QStyledItemDelegate::setEditorData(editor, index);
    return;
}

void NameServiceItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    NameServiceModel *nsmodel = static_cast<NameServiceModel *>(model);

    QStandardItem *item = nsmodel->itemFromIndex(index);

    TypedItem *ti = dynamic_cast<TypedItem*>(item);
    if (!ti)
    {
        QStyledItemDelegate::setModelData(editor, model, index);
        return;
    }

    ItemBase* itembase = nullptr;
    if (ti->type() == ItemType::INPUTPORT)
        itembase = static_cast<PortItem *>(ti->getData())->getItemBase().get();
    else if (ti->type() == ItemType::EDITABLEITEM)
        itembase = static_cast<ItemBase *>(ti->getData());
    else
    {
        QStyledItemDelegate::setModelData(editor, model, index);
        return;
    }

    for (auto &h : itembase->getHandlerStack())
    {
        if (h->flags() & ConfigItemHandler::Flags::CustomEditor)
        {
            if (h->setModelData(editor, nsmodel, index))
            {
                nsmodel->notifyItemDataEdited(index);
            }
            return;
        }
        if (h->flags() & ConfigItemHandler::Flags::ConvertsToTypelibValue)
        {
            QStyledItemDelegate::setModelData(editor, model, index);
            if (h->convertToTypelibValue(itembase->getValueHandle(), item, itembase->codec))
            {
                nsmodel->notifyItemDataEdited(index);
            }
            return;
        }
    }

    QStyledItemDelegate::setModelData(editor, model, index);

    auto es = dynamic_cast<EditableSimple*>(itembase);
    if (es)
    {
        if (es->updateFromEdit())
        {
            nsmodel->notifyItemDataEdited(index);
        }
    }
}

void NameServiceItemDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyledItemDelegate::updateEditorGeometry(editor, option, index);
}

#pragma once

#include <QCoreApplication>
#include "../ConfigItemHandler.hpp"

class BoolHandler : public ConfigItemHandler
{
    Q_DECLARE_TR_FUNCTIONS(EnumHandler)
public:
    virtual int flags() const override { return CustomEditor | ConvertsFromTypelibValue; }

    virtual QWidget *createEditor(NameServiceItemDelegate const* delegate, QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    virtual void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    virtual bool setModelData(QWidget *editor, NameServiceModel *model, const QModelIndex &index) const override;
    virtual void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    virtual bool convertFromTypelibValue(QStandardItem *dst, Typelib::Value const &src, QTextCodec *codec) const override;

    virtual bool convertToTypelibValue(Typelib::Value &dst, QStandardItem *src, QTextCodec *codec) const override;

    virtual bool probe(Typelib::Type const &type, bool editing = true) const override;
};

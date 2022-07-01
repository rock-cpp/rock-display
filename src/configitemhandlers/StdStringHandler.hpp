
#pragma once

#include <QCoreApplication>
#include "../ConfigItemHandler.hpp"

class StdStringHandler : public ConfigItemHandler
{
    Q_DECLARE_TR_FUNCTIONS(StdStringHandler)
public:
    virtual int flags() const override { return HideFields | ConvertsFromTypelibValue | ConvertsToTypelibValue; }

    virtual bool convertFromTypelibValue(QStandardItem *dst, Typelib::Value const &src, QTextCodec *codec) const override;

    virtual bool convertToTypelibValue(Typelib::Value &dst, QStandardItem *src, QTextCodec *codec) const override;

    virtual bool probe(Typelib::Type const &type, bool editing = true) const override;
};

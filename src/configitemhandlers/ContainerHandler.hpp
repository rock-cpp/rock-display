#pragma once

#include <QCoreApplication>
#include "../ConfigItemHandler.hpp"

class ContainerHandler : public ConfigItemHandler
{
    Q_DECLARE_TR_FUNCTIONS(ContainerHandler)
public:
    virtual int flags() const override { return ContextMenuItems; }

    virtual bool addContextMenuEntries ( QMenu * menu, const QModelIndex & index ) const override;

    virtual bool probe(Typelib::Type const &type, bool editing = true) const override;
};

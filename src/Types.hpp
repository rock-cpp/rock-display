#pragma once

#include <QObject>
#include "Vizkit3dPluginRepository.hpp"

enum ItemType {TASK          = QStandardItem::UserType+1,   //1001
               OUTPUTPORT    = QStandardItem::UserType+2,
               INPUTPORT     = QStandardItem::UserType+3,
               CONFIGITEM    = QStandardItem::UserType+4,
               NAMESERVICE   = QStandardItem::UserType+5 };

class TypedItem;

class DataContainer : public QObject {
    Q_OBJECT

    private:
        PluginHandle _handle;
        TypedItem *_ti;

    public:
        DataContainer(PluginHandle handle, TypedItem *ti) : _handle(handle), _ti(ti)
        {

        }

        virtual ~DataContainer()
        {

        }

        PluginHandle getPluginHandle()
        {
            return _handle;
        }

        TypedItem* getItem()
        {
            return _ti;
        }
};
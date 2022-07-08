#pragma once

#include <QObject>
#include <QStandardItem>

enum ItemType {TASK          = QStandardItem::UserType+1,   //1001
               OUTPUTPORT    = QStandardItem::UserType+2,
               INPUTPORT     = QStandardItem::UserType+3,
               CONFIGITEM    = QStandardItem::UserType+4,
               NAMESERVICE   = QStandardItem::UserType+5,
               EDITABLEITEM  = QStandardItem::UserType+6,
               PROPERTYITEM  = QStandardItem::UserType+7 };

class TypedItem;
class PluginHandle;

class DataContainer : public QObject {
    Q_OBJECT

    private:
        PluginHandle const *_handle;
        TypedItem *_ti;

    public:
        DataContainer(PluginHandle const *handle, TypedItem *ti) : _handle(handle), _ti(ti)
        {

        }

        virtual ~DataContainer()
        {

        }

        PluginHandle const *getPluginHandle()
        {
            return _handle;
        }

        TypedItem* getItem()
        {
            return _ti;
        }
};

#pragma once

#include <QObject>
#include "Vizkit3dPluginRepository.hpp"

enum ItemType {TASK = 1001, OUTPUTPORT = 1002, INPUTPORT = 1003, CONFIGITEM = 1004, NAMESERVICE = 1005 };

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
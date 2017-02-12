#pragma once

#include <QObject>
#include "Vizkit3dPluginRepository.hpp"
#include "PortItem.hpp"

enum ItemType {TASK = 1001, OUTPUTPORT = 1002, INPUTPORT = 1003, CONFIGITEM = 1004, NAMESERVICE = 1005, PROPERTY = 1006};

class ItemBase;

class DataContainer : public QObject {
    Q_OBJECT

    private:
        PluginHandle _handle;
        ItemBase *_opi;

    public:
        DataContainer(PluginHandle handle, ItemBase *opi) : _handle(handle), _opi(opi)
        {

        }

        virtual ~DataContainer()
        {

        }

        PluginHandle getPluginHandle()
        {
            return _handle;
        }

        ItemBase* getItem()
        {
            return _opi;
        }
};
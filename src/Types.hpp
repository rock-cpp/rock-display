#pragma once

#include <QObject>
#include "Vizkit3dPluginRepository.hpp"
#include "PortItem.hpp"

enum ItemType {TASK = 1001, OUTPUTPORT = 1002, INPUTPORT = 1003};

class OutputPortItem;

class DataContainer : public QObject {
    Q_OBJECT

    private:
        PluginHandle _handle;
        OutputPortItem *_opi;

    public:
        DataContainer(PluginHandle handle, OutputPortItem *opi) : _handle(handle), _opi(opi)
        {

        }

        virtual ~DataContainer()
        {

        }

        PluginHandle getPluginHandle()
        {
            return _handle;
        }

        OutputPortItem* getOutputPortItem()
        {
            return _opi;
        }
};

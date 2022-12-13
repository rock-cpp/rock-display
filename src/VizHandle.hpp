
#pragma once

#include <QObject>

namespace Typelib
{
    class Value;
}

class VizHandle : public QObject
{
    Q_OBJECT
public:
    virtual ~VizHandle() {}
public slots:
    virtual void updateVisualizer(Typelib::Value const &value){}
    /* this sample can be kept around for editing purposes */
    virtual void updateEditable(Typelib::Value const &value){}
signals:
    void editableChanged(Typelib::Value const &value, bool force_send = false);
    void closing(VizHandle *vh);
};


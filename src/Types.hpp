#pragma once

#include <QObject>

enum ItemType {TASK = 1001, PORT = 1002};

class SignalInformation: public QObject {
    Q_OBJECT

    private:
        int _cmd;
        QString _name;
    public:
        enum CmdType {ACTIVATE = 0, START = 1, STOP = 2, CONFIGURE = 3};

        SignalInformation(int cmd, const QString &name) : _cmd(cmd), _name(name)
        {

        }

        virtual ~SignalInformation() {

        }

        QString name() {
            return _name;
        }

        int cmd() {
            return _cmd;
        }
};

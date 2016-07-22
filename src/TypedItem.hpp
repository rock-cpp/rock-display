#pragma once

#include <QStandardItem>

class TypedItem : public QStandardItem
{

private:
    int _type;

public:
    explicit TypedItem(int type);
    TypedItem();
    void setType(int newType);
    virtual int type();
};

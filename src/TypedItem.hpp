#pragma once

#include <QStandardItem>

class TypedItem : public QStandardItem
{

private:
    int _type;
    void *userData;
    
public:
    explicit TypedItem(int type);
    TypedItem();
    virtual ~TypedItem();
    void setType(int newType);
    
    void setData(void *data);
    void *getData();
    
    virtual int type();
};

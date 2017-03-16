#pragma once

#include <QStandardItem>

class TypedItem : public QStandardItem
{

private:
    int _type;
    void *userData;
    bool expanded;
    
public:
    explicit TypedItem(int type);
    TypedItem();
    virtual ~TypedItem();
    void setType(int newType);
    
    void setData(void *data);
    void *getData();
    
    virtual int type();
    
    virtual bool operator<(const QStandardItem &other) const;
    
    void setExpanded(bool expanded)
    {
        this->expanded = expanded;
    }
    
    bool isExpanded()
    {
        return this->expanded;
    }
};
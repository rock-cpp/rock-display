#include "TypedItem.hpp"
#include <iostream>

TypedItem::TypedItem(int type) : QStandardItem(nullptr), _type(type), userData(nullptr)
{
    setEditable(false);
    setText("");
}

TypedItem::TypedItem() : QStandardItem(nullptr), _type(-1), userData(nullptr)
{
    setEditable(false);
    setText("");
}

void TypedItem::setData(void* data)
{
    userData = data;
}

void* TypedItem::getData()
{
    if(!userData)
        throw std::runtime_error("Data is zero");
    return userData;
}

void TypedItem::setType(int newType)
{
    _type = newType;
}

int TypedItem::type()
{
    if(_type == -1)
        return QStandardItem::type();

    return _type;
}

TypedItem::~TypedItem()
{
}
#include "TypedItem.hpp"

TypedItem::TypedItem(int type) : QStandardItem(), _type(type), userData(nullptr)
{
    setEditable(false);
}

TypedItem::TypedItem() : _type(-1), userData(nullptr)
{
    setEditable(false);
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

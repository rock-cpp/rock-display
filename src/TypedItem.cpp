#include "TypedItem.hpp"

TypedItem::TypedItem(int type) : QStandardItem(), _type(type)
{
    isEditable(false);
}

TypedItem::TypedItem() : _type(-1)
{

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

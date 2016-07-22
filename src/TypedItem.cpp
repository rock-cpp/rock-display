#include "TypedItem.hpp"

TypedItem::TypedItem(int type) : QStandardItem(), _type(type)
{

}

TypedItem::TypedItem() : _type(-1)
{

}

void TypedItem::setType(int newType)
{

}

int TypedItem::type()
{
    if(_type == -1)
        return QStandardItem::type();
    
    return _type;
}

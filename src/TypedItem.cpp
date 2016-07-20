#include "TypedItem.hpp"

TypedItem::TypedItem(int type) : QStandardItem(), _type(type)
{

}

int TypedItem::type()
{
    return _type;
}

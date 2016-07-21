#include "ConfigItem.hpp"

ConfigItem::ConfigItem()
{

}

QList< QStandardItem* > ConfigItem::getValue(const std::shared_ptr< libConfig::ConfigValue >& value)
{
    switch(value->getType())
    {
        case libConfig::ConfigValue::ARRAY:
            return getArray(value);
            break;
        case libConfig::ConfigValue::COMPLEX:
            return getComplex(value);
            break;
        case libConfig::ConfigValue::SIMPLE:
            return getSimple(value);
            break;
    }
    
    std::cout << "ERROR" << std::endl;
    
    return {};
}

QList< QStandardItem* > ConfigItem::getComplex(const std::shared_ptr< libConfig::ConfigValue >& value)
{
    libConfig::ComplexConfigValue *cVal = static_cast<libConfig::ComplexConfigValue *>(value.get());
    QStandardItem *topName = new QStandardItem(cVal->getName().c_str());
    QStandardItem *typeName = new QStandardItem(cVal->getCxxTypeName().c_str());
    
    for(auto &v : cVal->getValues())
    {
        auto list = getValue(v.second);
        list.front()->setText(v.first.c_str());
//         list.insert(0, new QStandardItem(v.second->getName().c_str()));
        topName->appendRow( list );
    }
    
    return {topName, typeName};
}

QList< QStandardItem* > ConfigItem::getSimple(const std::shared_ptr< libConfig::ConfigValue >& value)
{
    libConfig::SimpleConfigValue *sVal = static_cast<libConfig::SimpleConfigValue *>(value.get());
    
    return {new QStandardItem(sVal->getName().c_str()), new QStandardItem(sVal->getValue().c_str())};
}

QList< QStandardItem* > ConfigItem::getArray(const std::shared_ptr< libConfig::ConfigValue >& value)
{
    libConfig::ArrayConfigValue *aVal = static_cast<libConfig::ArrayConfigValue *>(value.get());
    QStandardItem *topName = new QStandardItem(aVal->getName().c_str());
    QStandardItem *typeName = new QStandardItem(aVal->getCxxTypeName().c_str());
    
    size_t cnt = 0;
    for(auto &v : aVal->getValues())
    {
        auto list = getValue(v);
        list.front()->setText(QString::number(cnt));
        topName->appendRow(list);
        cnt++;
    }
    
    return {topName, typeName};
}

void ConfigItem::update(const std::shared_ptr< libConfig::ConfigValue >& value, QStandardItem* parent)
{
    parent->removeColumns(0, parent->columnCount());
    parent->removeRows(0, parent->rowCount());

    parent->appendRow(getValue(value));
}

// QList< QStandardItem* >  ConfigItem::update(std::shared_ptr< libConfig::ConfigValue >& value)
// {
//     removeColumns(0, columnCount());
//     removeRows(0, rowCount());
//     
//     
// }

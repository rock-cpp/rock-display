#pragma once

#include <QStandardItem>
#include <lib_config/Configuration.hpp>



class ConfigItem
{
    std::vector<QStandardItem *> items;
public:
    ConfigItem();
    
    QList<QStandardItem *> getValue(const std::shared_ptr< libConfig::ConfigValue >& value);
    QList<QStandardItem *> getComplex(const std::shared_ptr< libConfig::ConfigValue >& value);
    QList<QStandardItem *> getSimple(const std::shared_ptr< libConfig::ConfigValue >& value);
    QList<QStandardItem *> getArray(const std::shared_ptr< libConfig::ConfigValue >& value);

    
    void update(const std::shared_ptr< libConfig::ConfigValue >& value, QStandardItem* parent);
    
};



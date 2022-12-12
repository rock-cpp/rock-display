
#pragma once

#include <string>
#include <memory>

class TypeConverter;

class TypeConverterFactory {
public:
    virtual ~TypeConverterFactory() {}
    virtual std::string getResultTypename() const = 0;
    virtual std::unique_ptr<TypeConverter> createConverter() const = 0;
};



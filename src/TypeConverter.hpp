
#pragma once

#include <string>
#include <typelib/value.hh>

class TypeConverter {
public:
    struct ConversionResult {
        Typelib::Value convertedValue;
        void *convertedRawPtr;
    };
    virtual ~TypeConverter() {}
    virtual std::string getResultTypename() const = 0;
    virtual ConversionResult convertToResult(Typelib::Value const &value,
            const Typelib::Registry *registry) = 0;
    virtual void refreshFromResult(Typelib::Value &orig_value) = 0;
};



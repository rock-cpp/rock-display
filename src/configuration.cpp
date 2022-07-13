
#include "configuration.hpp"

#include <yaml-cpp/yaml.h>
#include <typelib/value.hh>
#include <fstream>
#include <QString>

#include "TaskItem.hpp"
#include "PropertyItem.hpp"

/* This is here because lib_config does not have any facilities to save to a file.
 * There is orocos_cpp::ConfigurationHelper::getYamlString, but it a) creates a string
 * already which we would have to parse into yaml nodes again just to have them
 * converted to strings shortly after and b) it does not handle 8 bit entities and bools right.
 */

YAML::Node yamlNodeFromTypelibValue(const Typelib::Value &value);

YAML::Node yamlNodeFromTypelibValue(const Typelib::Numeric &type, const Typelib::Value &value){
    if (type.getName() == "/bool")
    {
        return YAML::Node(*static_cast<bool *>(value.getData())?"true":"false");
    }
    switch(type.getNumericCategory())
    {
    case Typelib::Numeric::Float:
        if(type.getSize() == sizeof(float))
        {
            return YAML::Node(*static_cast<float *>(value.getData()));
        }
        else
        {
            return YAML::Node(*static_cast<double *>(value.getData()));
        }
    case Typelib::Numeric::SInt:
        switch(type.getSize())
        {
        case sizeof(int8_t):
            return YAML::Node(static_cast<int>(*static_cast<int8_t *>(value.getData())));
        case sizeof(int16_t):
            return YAML::Node(*static_cast<int16_t *>(value.getData()));
        case sizeof(int32_t):
            return YAML::Node(*static_cast<int32_t *>(value.getData()));
        case sizeof(int64_t):
            return YAML::Node(*static_cast<int64_t *>(value.getData()));
        default:
            std::cerr << "Error, got integer of unexpected size " << type.getSize() << std::endl;
            throw std::runtime_error("got integer of unexpected size");
        }
    case Typelib::Numeric::UInt:
    {
        switch(type.getSize())
        {
        case sizeof(uint8_t):
            return YAML::Node(static_cast<unsigned int>(*static_cast<uint8_t *>(value.getData())));
        case sizeof(uint16_t):
            return YAML::Node(*static_cast<uint16_t *>(value.getData()));
        case sizeof(uint32_t):
            return YAML::Node(*static_cast<uint32_t *>(value.getData()));
        case sizeof(uint64_t):
            return YAML::Node(*static_cast<uint64_t *>(value.getData()));
        default:
            std::cout << "Error, got integer of unexpected size " << type.getSize() << std::endl;
            throw std::runtime_error("got integer of unexpected size");
        }
    }
    case Typelib::Numeric::NumberOfValidCategories:
    default:
        throw std::runtime_error("Internal Error: Got invalid Category");
    }
}

YAML::Node yamlNodeFromTypelibValue(const Typelib::Container &type, const Typelib::Value &value){

    const size_t size = type.getElementCount(value.getData());

    if(type.kind() == "/std/string")
    {
        const std::string *content = static_cast<const std::string *>(value.getData());

        return YAML::Node(*content);
    }

    //std::vector
    YAML::Node result(YAML::NodeType::Sequence);
    for(size_t i = 0; i < size; i++)
    {
        Typelib::Value elem = type.getElement(value.getData(), i);
        result.push_back(yamlNodeFromTypelibValue(elem));
    }

    return result;
}

YAML::Node yamlNodeFromTypelibValue(const Typelib::Compound &type, const Typelib::Value &value){
    YAML::Node result(YAML::NodeType::Map);

    uint8_t *data = static_cast<uint8_t *>( value.getData());

    Typelib::Compound::FieldList const fields = type.getFields();
    for(const Typelib::Field &field: fields)
    {
        Typelib::Value fieldV(data + field.getOffset(), field.getType());
        result[field.getName()] = yamlNodeFromTypelibValue(fieldV);
    }

    return result;
}

YAML::Node yamlNodeFromTypelibValue(const Typelib::Array &type, const Typelib::Value &value){
    YAML::Node result(YAML::NodeType::Sequence);

    const Typelib::Type &indirect(type.getIndirection());
    for(size_t i = 0; i < type.getDimension(); i++)
    {
        Typelib::Value arrayV(static_cast<uint8_t *>(value.getData()) + i * indirect.getSize(), indirect);
        result.push_back(yamlNodeFromTypelibValue(arrayV));
    }

    return result;
}

YAML::Node yamlNodeFromTypelibValue(const Typelib::Enum &type, const Typelib::Value &value){
    Typelib::Enum::integral_type *intVal = (static_cast<Typelib::Enum::integral_type *>(value.getData()));
    return YAML::Node(std::string(":")+type.get(*intVal));
}

YAML::Node yamlNodeFromTypelibValue(const Typelib::Value &value)
{
    const Typelib::Type& type = value.getType();
    if(type.getCategory() == Typelib::Type::Array){
        const Typelib::Array &array = static_cast<const Typelib::Array &>(value.getType());
        return yamlNodeFromTypelibValue(array, value);
    }
    else if(type.getCategory() == Typelib::Type::Compound){
        const Typelib::Compound &compound = static_cast<const Typelib::Compound &>(value.getType());
        return yamlNodeFromTypelibValue(compound, value);
    }
    else if(type.getCategory() == Typelib::Type::Container){
        const Typelib::Container &container = static_cast<const Typelib::Container &>(value.getType());
        return yamlNodeFromTypelibValue(container, value);
    }
    else if(type.getCategory() == Typelib::Type::Enum){
        const Typelib::Enum &en = static_cast<const Typelib::Enum &>(value.getType());
        return yamlNodeFromTypelibValue(en, value);
    }
    else if(type.getCategory() == Typelib::Type::Numeric){
        const Typelib::Numeric &num = static_cast<const Typelib::Numeric &>(value.getType());
        return yamlNodeFromTypelibValue(num, value);
    }
    else if(type.getCategory() == Typelib::Type::NullType){
        throw std::runtime_error("Got Unsupported Category: NullType");
    }
    else if(type.getCategory() == Typelib::Type::Opaque){
        throw std::runtime_error("Got Unsupported Category: Opaque");
    }
    else if(type.getCategory() == Typelib::Type::Pointer){
        throw std::runtime_error("Got Unsupported Category: Pointer");
    }
    else{
        throw std::runtime_error(std::string("Got Unexpected Category: ")+std::to_string(type.getCategory()));
    }
}

bool save_configuration(RTT::TaskContext* task, TaskItem *titem, std::string const &filename)
{
    std::ofstream out(filename, std::ios_base::trunc);
    out << "--- name:default\n";

    RTT::PropertyBag *taskProperties = task->properties();
    auto &propertyitems = titem->getProperties();

    for (std::size_t i=0; i<taskProperties->size(); i++)
    {
        RTT::base::PropertyBase *property = taskProperties->getItem(i);
        auto it = propertyitems.find(property->getName());
        if (it == propertyitems.end())
        {
            continue;
        }
        QString desc = QString::fromStdString(property->getDescription());
        if(!desc.isEmpty())
        {
            desc.replace("\n","\n# ");
            out << "# " << desc.toStdString() << "\n";
        }
        YAML::Node node(YAML::NodeType::Map);
        YAML::Node pn = yamlNodeFromTypelibValue(it->second->getValueHandle());
        node[it->first] = pn;
        out << node << "\n";
    }

    out.close();
    return true;
}


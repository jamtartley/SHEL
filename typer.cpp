#include "typer.hpp"

std::string data_type_to_string(Data_Type type) {
    switch (type) {
        case Data_Type::NUM:     return "num";
        case Data_Type::STR:     return "str";
        case Data_Type::BOOL:    return "bool";
        case Data_Type::UNKNOWN: return "unknown";
        default:                 return "";
    }
}

#include "typer.hpp"

std::string data_type_to_string(Data_Type type) {
    switch (type) {
        case Data_Type::NUM:     return "num";
        case Data_Type::STR:     return "str";
        case Data_Type::BOOL:    return "bool";
        case Data_Type::VOID:    return "void";
        case Data_Type::UNKNOWN: return "unknown";
        default:                 return "";
    }
}

Data_Type token_to_data_type(Token *token) {
    switch (token->type) {
        case Token::Type::KEYWORD_NUM:  return Data_Type::NUM;
        case Token::Type::KEYWORD_STR:  return Data_Type::STR;
        case Token::Type::KEYWORD_BOOL: return Data_Type::BOOL;
        case Token::Type::KEYWORD_VOID: return Data_Type::VOID;
        default:                        return Data_Type::UNKNOWN;
    }
}

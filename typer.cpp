#include "typer.hpp"

std::string data_type_to_string(Data_Type type) {
    switch (type) {
        case Data_Type::NUM:     return "num";
        case Data_Type::STR:     return "str";
        case Data_Type::BOOL:    return "bool";
        case Data_Type::VOID:    return "void";
        default:                 return "";
    }
}

std::string atom_to_string(Data_Atom *atom) {
    switch (atom->data_type) {
        case Data_Type::NUM: {
            const unsigned int dp_count = 2;
            float value = ((Num_Atom *)atom)->value;
            std::string base_string = std::to_string(value);
            int dp_index = base_string.find(".");
            std::string from_decimal_point = base_string.substr(dp_index);
            bool is_all_zeroes = true;

            for (int i = 1; i < from_decimal_point.size(); i++) {
                if (from_decimal_point[i] != '0') {
                    is_all_zeroes = false;
                    break;
                }
            }

            if (is_all_zeroes) return base_string.substr(0, dp_index);
            return dp_index + dp_count + 1 > base_string.size() ? base_string : base_string.substr(0, dp_index + dp_count + 1);
        }
        case Data_Type::STR:     return ((Str_Atom *)atom)->value;
        case Data_Type::BOOL:    return ((Bool_Atom *)atom)->value ? "true" : "false";
        default:                 return "";
    }
}

Data_Type token_to_data_type(Token *token) {
    switch (token->type) {
        case Token::Type::KEYWORD_NUM:  return Data_Type::NUM;
        case Token::Type::KEYWORD_STR:  return Data_Type::STR;
        case Token::Type::KEYWORD_BOOL: return Data_Type::BOOL;
        default:
        case Token::Type::KEYWORD_VOID: return Data_Type::VOID;
    }
}

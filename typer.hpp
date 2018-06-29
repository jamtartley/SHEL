#ifndef TYPER_H
#define TYPER_H

#include <string>
#include "lexer.hpp"

enum Data_Type {
    NUM,
    STR,
    BOOL,
    VOID,
    UNKNOWN
};

struct Data_Value {
    float num_val;
    std::string str_val;
    bool bool_val;

    Data_Type data_type = Data_Type::UNKNOWN;

    Data_Value() {
        this->data_type = Data_Type::VOID;
    }

    Data_Value(float num_val) {
        this->num_val = num_val;
        this->data_type = Data_Type::NUM;
    }

    Data_Value(std::string str_val) {
        this->str_val = str_val;
        this->data_type = Data_Type::STR;
    }

    Data_Value(bool bool_val) {
        this->bool_val = bool_val;
        this->data_type = Data_Type::BOOL;
    }
};

std::string data_type_to_string(Data_Type type);
Data_Type token_to_data_type(Token *token);

#endif

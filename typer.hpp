#ifndef TYPER_H
#define TYPER_H

#include <string>
#include <vector>
#include "lexer.hpp"

enum Data_Type {
    NUM,
    STR,
    BOOL,
    ARRAY,
    VOID,
    UNKNOWN
};

struct Data_Atom {
    Data_Type data_type = Data_Type::UNKNOWN;
};

struct Num_Atom : Data_Atom {
    float value;

    Num_Atom(float value) {
        this->value = value;
        this->data_type = Data_Type::NUM;
    }
};

struct Str_Atom : Data_Atom {
    std::string value;

    Str_Atom(std::string value) {
        this->value = value;
        this->data_type = Data_Type::STR;
    }
};

struct Bool_Atom : Data_Atom {
    bool value;

    Bool_Atom(bool value) {
        this->value = value;
        this->data_type = Data_Type::BOOL;
    }
};

struct Array_Atom : Data_Atom  {
    std::vector<Data_Atom *> value;

    Array_Atom() {
        this->data_type = Data_Type::ARRAY;
    }
};

std::string data_type_to_string(Data_Type type);
Data_Type token_to_data_type(Token *token);

#endif

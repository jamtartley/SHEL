#ifndef TYPER_H
#define TYPER_H

enum Data_Type {
    NUM,
    STR,
    BOOL,
    UNKNOWN
};

struct Data_Value {
    float num_val;
    std::string str_val;
    bool bool_val;

    Data_Type data_type = Data_Type::UNKNOWN;

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

#endif

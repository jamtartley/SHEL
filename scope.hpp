#ifndef SCOPE_H
#define SCOPE_H

#include <map>

struct Scope {
    int depth; // @CLEANUP Does scope need depth?
    Scope *parent;
    std::map<std::string, std::string> variables;

    Scope(Scope *parent) {
        this->parent = parent;
        this->depth = parent == nullptr ? 0 : parent->depth + 1;
    }
};

struct Var_With_Success {
    std::string str_value;
    float num_value;
    bool was_success;

    Var_With_Success(std::string str_value, bool was_success) {
        this->str_value = str_value;
        this->num_value = std::atof(str_value.c_str());
        this->was_success = was_success;
    }
};

Var_With_Success *get_var(Scope *scope, std::string name); 
void set(Scope *scope, std::string name, std::string value); 
bool is_in_scope(Scope *scope, std::string name);
void print_contents(Scope *scope);

#endif

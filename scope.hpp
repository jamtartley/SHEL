#ifndef SCOPE_H
#define SCOPE_H

#include <map>
#include "parser.hpp"
#include "typer.hpp"

struct Scope {
    int depth; // @CLEANUP Does scope need depth?
    bool is_global_scope;
    Scope *parent;
    std::map<std::string, Data_Value *> variables;
    std::map<std::string, Ast_Function_Definition *> functions;

    Scope(Scope *parent) {
        this->parent = parent;
        this->depth = parent == NULL ? 0 : parent->depth + 1;
        this->is_global_scope = depth == 0;
    }
};

struct Func_With_Success {
    Ast_Function_Definition *func_def;
    bool was_success;

    Func_With_Success(Ast_Function_Definition *func_def, bool was_success) {
        this->func_def = func_def;
        this->was_success = was_success;
    }
};

struct Var_With_Success {
    Data_Value *data;
    bool was_success;

    Var_With_Success(Data_Value *data, bool was_success) {
        this->data = data;
        this->was_success = was_success;
    }
};

Var_With_Success *get_var(Scope *scope, std::string name);
Func_With_Success *get_func(Scope *scope, std::string name);
void assign_var(Scope *scope, std::string name, Data_Value *value);
void reassign_var(Scope *scope, std::string name, Data_Value *value, Code_Site *site);
void set_func(Scope *scope, std::string name, Ast_Function_Definition *func);
bool is_var_in_scope(Scope *scope, std::string name);
bool is_func_in_scope(Scope *scope, std::string name);
void print_contents(Scope *scope);

#endif

#ifndef INTERP_H
#define INTERP_H

#include <map>
#include "parser.hpp"
#include "scope.hpp"

struct Interpreter {
    Parser *parser;

    Interpreter(Parser *parser) {
        this->parser = parser;
    }
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

Data_Value *walk_block_node(Interpreter *interp, Scope *scope, Ast_Block *root);

Data_Value *walk_expression(Interpreter *interp, Scope *scope, Ast_Node *node);
Data_Value *walk_binary_op_node(Interpreter *interp, Scope *scope, Ast_Binary_Op *node);
Data_Value *walk_unary_op_node(Interpreter *interp, Scope *scope, Ast_Unary_Op *node);

float get_number_literal(Scope *scope, Ast_Literal *node);
float get_number_variable(Scope *scope, Ast_Variable *node);

bool evaluate_comparison(Interpreter *interp, Scope *scope, Ast_Comparison *comparison);

std::string get_string_literal(Scope *scope, Ast_Literal *node);
std::string get_string_from_return_value(Data_Value *ret);

bool is_num(std::string str);

void add_function_def_to_scope(Interpreter *interp, Scope *scope, Ast_Function_Definition *def);
void fail_if_binary_op_invalid(Data_Value *left, Data_Value *right, Token *op);

Data_Value *walk_function_call(Interpreter *interp, Scope *scope, Ast_Function_Call *call);
Data_Value *walk_if(Interpreter *interp, Scope *scope, Ast_If *if_node);
Data_Value *walk_while(Interpreter *interp, Scope *scope, Ast_While *while_node);
Data_Value *walk_loop(Interpreter *interp, Scope *scope, Ast_Loop *loop_node);
Data_Value *walk_from_root(Interpreter *interp, Scope *scope, Ast_Node *root);

void call_native_function(Interpreter *interp, Scope *scope, Ast_Function_Call *call);
void interpret(Interpreter *interp);

#endif

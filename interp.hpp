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

struct Return_Value {
    float num_val;
    std::string str_val;
    bool bool_val;

    Return_Value(float num_val) {
        this->num_val = num_val;
    }
};

Return_Value *walk_block_node(Interpreter *interp, Scope *scope, Ast_Block *root);

float walk_from_arithmetic_root(Interpreter *interp, Scope *scope, Ast_Node *node);
float walk_binary_op_node(Interpreter *interp, Scope *scope, Ast_Binary_Op *node);
float walk_unary_op_node(Interpreter *interp, Scope *scope, Ast_Unary_Op *node);
float return_node_to_float(Interpreter *interp, Scope *scope, Return_Value *return_node);

float get_number_literal(Scope *scope, Ast_Literal *node);
float get_number_variable(Scope *scope, Ast_Variable *node);

bool evaluate_comparison(Interpreter *interp, Scope *scope, Ast_Comparison *comparison);

std::string get_string_literal(Scope *scope, Ast_Literal *node);
std::string get_string_variable(Interpreter *interp, Scope *scope, Ast_Variable *node);

bool is_num(std::string str);

void add_function_def_to_scope(Interpreter *interp, Scope *scope, Ast_Function_Definition *def);

Return_Value *walk_function_call(Interpreter *interp, Scope *scope, Ast_Function_Call *call);
Return_Value *walk_if(Interpreter *interp, Scope *scope, Ast_If *if_node);
Return_Value *walk_while(Interpreter *interp, Scope *scope, Ast_While *while_node);
Return_Value *walk_from_root(Interpreter *interp, Scope *scope, Ast_Node *root);

void call_native_function(Interpreter *interp, Scope *scope, Ast_Function_Call *call);
void interpret(Interpreter *interp);

#endif

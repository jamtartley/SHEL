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

float walk_unary_op_node(Interpreter *interp, Scope *scope, Unary_Op_Node *node);
float walk_binary_op_node(Interpreter *interp, Scope *scope, Binary_Op_Node *node);
float walk_number_node(Scope *scope, Number_Node *node);
float walk_from_arithmetic_root(Interpreter *interp, Scope *scope, Ast_Node *node);
float walk_num_variable_node(Interpreter *interp, Scope *scope, Variable_Node *node);
std::string walk_str_variable_node(Interpreter *interp, Scope *scope, Variable_Node *node);
void walk_from_root(Scope *scope, Ast_Node *root);
float walk_num_function_node(Interpreter *interp, Scope *scope, Function_Definition_Node *def);
std::string walk_str_function_node(Interpreter *interp, Scope *scope, Function_Definition_Node *def);
void add_function_def_to_scope(Interpreter *interp, Scope *scope, Function_Definition_Node *def);
void walk_function_call(Interpreter *interp, Scope *scope, Function_Call_Node *call);
void walk_block_node(Interpreter *interp, Scope *scope, Block_Node *root);
void walk_empty_node(Scope *scope, Empty_Node *node);
void interpret(Interpreter *interp);

#endif

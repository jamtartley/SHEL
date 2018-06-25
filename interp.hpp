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

Ast_Return *walk_block_node(Interpreter *interp, Scope *scope, Ast_Block *root);

float walk_from_arithmetic_root(Interpreter *interp, Scope *scope, Ast_Node *node);
float walk_binary_op_node(Interpreter *interp, Scope *scope, Ast_Binary_Op *node);
float walk_unary_op_node(Interpreter *interp, Scope *scope, Ast_Unary_Op *node);

float get_number_literal(Scope *scope, Ast_Literal *node);
float get_number_variable(Scope *scope, Ast_Variable *node);

std::string get_string_literal(Scope *scope, Ast_Literal *node);
std::string get_string_variable(Interpreter *interp, Scope *scope, Ast_Variable *node);

bool is_num(std::string str);

void add_function_def_to_scope(Interpreter *interp, Scope *scope, Ast_Function_Definition *def);

Ast_Return *walk_function_call(Interpreter *interp, Scope *scope, Ast_Function_Call *call);
void walk_from_root(Scope *scope, Ast_Node *root);

void interpret(Interpreter *interp);

#endif

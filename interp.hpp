#ifndef INTERP_H
#define INTERP_H

#include <map>
#include "parser.hpp"
#include "scope.hpp"
#include "typer.hpp"

struct Interpreter {
    Parser *parser;

    Interpreter(Parser *parser) {
        this->parser = parser;
    }
};

Data_Value *walk_block_node(Interpreter *interp, Scope *scope, Ast_Block *root);

Data_Value *walk_expression(Interpreter *interp, Scope *scope, Ast_Node *node);
Data_Value *walk_binary_op_node(Interpreter *interp, Scope *scope, Ast_Binary_Op *node);
Data_Value *walk_unary_op_node(Interpreter *interp, Scope *scope, Ast_Unary_Op *node);
Data_Value *get_variable(Interpreter *interp, Scope *scope, Ast_Variable *node);
Data_Value *get_data_from_literal(Interpreter *interp, Scope *scope, Ast_Literal *lit);

bool evaluate_node_to_bool(Interpreter *interp, Scope *scope, Ast_Node *node);

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

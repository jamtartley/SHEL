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

    Array_Atom *walk_array_node(Scope *scope, Ast_Array *array);
    Data_Atom *walk_block_node(Scope *scope, Ast_Block *root);
    Data_Atom *walk_expression(Scope *scope, Ast_Node *node);
    Data_Atom *walk_binary_op_node(Scope *scope, Ast_Binary_Op *node);
    Data_Atom *walk_unary_op_node(Scope *scope, Ast_Unary_Op *node);
    Data_Atom *walk_function_call(Scope *scope, Ast_Function_Call *call);
    Data_Atom *walk_if(Scope *scope, Ast_If *if_node);
    Data_Atom *walk_while(Scope *scope, Ast_While *while_node);
    Data_Atom *walk_loop(Scope *scope, Ast_Loop *loop_node);
    Data_Atom *walk_from_root(Scope *scope, Ast_Node *root);
    Data_Atom *get_variable(Scope *scope, Ast_Variable *node);
    Data_Atom *get_data_from_literal(Scope *scope, Ast_Literal *lit);

    bool evaluate_node_to_bool(Scope *scope, Ast_Node *node);
    bool evaluate_binary_op_to_bool(Scope *scope, Ast_Binary_Op *node);

    void walk_assignment_node(Scope *scope, Ast_Assignment *node);
    void add_function_def_to_scope(Scope *scope, Ast_Function_Definition *def);
    void interpret();
};

std::string get_string_from_return_value(Data_Atom *ret);
bool is_num(std::string str);
void fail_if_binary_op_invalid(Data_Atom *left, Data_Atom *right, Token *op);

#endif

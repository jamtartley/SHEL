#ifndef INTERP_H
#define INTERP_H

#include <map>

struct Interpreter {
    Parser *parser;
    std::map<std::string, std::string> variable_map;

    Interpreter(Parser *parser) {
        this->parser = parser;
    }
};

float walk_unary_op_node(Interpreter *interp, Unary_Op_Node *node);
float walk_binary_op_node(Interpreter *interp, Binary_Op_Node *node);
float walk_num_node(Number_Node *node);
float walk_from_arithmetic_root(Interpreter *interp, Ast_Node *node);
float walk_num_variable_node(Interpreter *interp, Variable_Node *node);
std::string walk_str_variable_node(Interpreter *interp, Variable_Node *node);
void walk_from_root(Ast_Node *root);
void walk_block_node(Interpreter *interp, Block_Node *root);
void walk_empty_node(Empty_Node *node);
void interpret(Interpreter *interp);

#endif

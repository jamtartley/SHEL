#ifndef INTERP_H
#define INTERP_H

struct Interpreter {
    Parser *parser;

    Interpreter(Parser *parser) {
        this->parser = parser;
    }
};

float visit_unary_op_node(Unary_Op_Node *node);
float visit_binary_op_node(Binary_Op_Node *node);
float visit_num_node(Number_Node *node);
float walk_from_root(Ast_Node *root);
float interpret(Interpreter *interp);

#endif

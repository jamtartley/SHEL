#include <iostream>
#include "lexer.h"
#include "parser.h"
#include "interp.h"

float visit_binary_op_node(Binary_Op_Node *node) {
    if (node->op->type == Token::Type::PLUS) {
        return walk_from_root(node->left) + walk_from_root(node->right);
    } else if (node->op->type == Token::Type::MINUS) {
        return walk_from_root(node->left) - walk_from_root(node->right);
    } else if (node->op->type == Token::Type::MULTIPLY) {
        return walk_from_root(node->left) * walk_from_root(node->right);
    } else {
        return walk_from_root(node->left) / walk_from_root(node->right);
    }
}

float visit_number_node(Number_Node *node) {
    return node->value;
}

float walk_from_root(Ast_Node *root) {
    if (root->node_type == Ast_Node::Type::BINARY_OP) {
        return visit_binary_op_node(static_cast<Binary_Op_Node *>(root));
    } else {
        return visit_number_node(static_cast<Number_Node *>(root));
    }
}

#include <iostream>
#include "lexer.h"
#include "parser.h"
#include "interp.h"

float walk_from_arithmetic_root(Ast_Node *node);
float walk_num_variable_node(Interpreter *interp, std::string name);
void walk_from_root(Interpreter *interp, Ast_Node *root);

float walk_unary_op_node(Interpreter *interp, Unary_Op_Node *node) {
    Token::Type type = node->op->type;

    if (type == Token::Type::OP_PLUS) {
        return +walk_from_arithmetic_root(interp, node->node);
    } else {
        return -walk_from_arithmetic_root(interp, node->node);
    }
}

float walk_binary_op_node(Interpreter *interp, Binary_Op_Node *node) {
    // @TODO(LOW) Handle string concatenation here
    if (node->op->type == Token::Type::OP_PLUS) {
        return walk_from_arithmetic_root(interp, node->left) + walk_from_arithmetic_root(interp, node->right);
    } else if (node->op->type == Token::Type::OP_MINUS) {
        return walk_from_arithmetic_root(interp, node->left) - walk_from_arithmetic_root(interp, node->right);
    } else if (node->op->type == Token::Type::OP_MULTIPLY) {
        return walk_from_arithmetic_root(interp, node->left) * walk_from_arithmetic_root(interp, node->right);
    } else {
        return walk_from_arithmetic_root(interp, node->left) / walk_from_arithmetic_root(interp, node->right);
    }
}

float walk_number_node(Number_Node *node) {
    return node->value;
}

float walk_from_arithmetic_root(Interpreter *interp, Ast_Node *node) {
    if (node->node_type == Ast_Node::Type::BINARY_OP) {
        return walk_binary_op_node(interp, static_cast<Binary_Op_Node *>(node));
    } else if (node->node_type == Ast_Node::Type::UNARY_OP) {
        return walk_unary_op_node(interp, static_cast<Unary_Op_Node *>(node));
    } else if (node->node_type == Ast_Node::Type::NUMBER) {
        return walk_number_node(static_cast<Number_Node *>(node));
    } else {
        return walk_num_variable_node(interp, static_cast<Variable_Node *>(node)->value);
    }
}

void walk_compound_node(Interpreter *interp, Compound_Node *root) {
    for (Ast_Node *child : root->statements) {
        walk_from_root(interp, child);
    }
}

void walk_empty_node(Empty_Node *node) {
    
}

float walk_num_variable_node(Interpreter *interp, std::string name) {
    if (interp->global_num_map.find(name) != interp->global_num_map.end()) {
        return interp->global_num_map[name];
    } else {
        std::cerr << "Use of unassigned num variable: " << name << std::endl;
        return 0;
    }
}

std::string walk_str_variable_node(Interpreter *interp, Variable_Node *node) {
    if (interp->global_str_map.find(node->value) != interp->global_str_map.end()) {
        return interp->global_str_map[node->value];
    } else {
        std::cerr << "Use of unassigned str variable: " << node->value << std::endl;
        return "";
    }
}

void walk_assignment_node(Interpreter *interp, Assignment_Node *node) {
    std::string name = node->left->value;
    Ast_Node::Type type = node->right->node_type;

    if (node->right->node_type == Ast_Node::Type::STRING) {
        interp->global_str_map[name] = static_cast<String_Node *>(node->right)->value;
    } else {
        interp->global_num_map[name] = walk_from_arithmetic_root(interp, node->right);
    }
}

void walk_from_root(Interpreter *interp, Ast_Node *root) {
    if (root->node_type == Ast_Node::Type::COMPOUND) {
        walk_compound_node(interp, static_cast<Compound_Node *>(root));
    } else if (root->node_type == Ast_Node::Type::EMPTY) {
        walk_empty_node(static_cast<Empty_Node *>(root));
    } else if (root->node_type == Ast_Node::Type::ASSIGNMENT) {
        walk_assignment_node(interp, static_cast<Assignment_Node *>(root));
    }
}

void interpret(Interpreter *interp) {
    Compound_Node *root = parse(interp->parser);
    walk_from_root(interp, root);

    for (auto const &k : interp->global_num_map) {
        std::cout << k.first << ": " << k.second << std::endl;
    }

    for (auto const &k : interp->global_str_map) {
        std::cout << k.first << ": " << k.second << std::endl;
    }
}

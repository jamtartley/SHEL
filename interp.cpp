#include <iostream>
#include "interp.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "scope.hpp"

float walk_from_arithmetic_root(Interpreter *interp, Scope *scope, Ast_Node *node);
float walk_number_variable_node(Interpreter *interp, Scope *scope, std::string name);
void walk_from_root(Interpreter *interp, Scope *scope,  Ast_Node *root);

float walk_unary_op_node(Interpreter *interp, Scope *scope, Unary_Op_Node *node) {
    Token::Type type = node->op->type;

    if (type == Token::Type::OP_PLUS) {
        return +walk_from_arithmetic_root(interp, scope, node->node);
    } else {
        return -walk_from_arithmetic_root(interp, scope, node->node);
    }
}

float walk_binary_op_node(Interpreter *interp, Scope *scope, Binary_Op_Node *node) {
    // @TODO(LOW) Handle string concatenation here
    if (node->op->type == Token::Type::OP_PLUS) {
        return walk_from_arithmetic_root(interp, scope, node->left) + walk_from_arithmetic_root(interp, scope, node->right);
    } else if (node->op->type == Token::Type::OP_MINUS) {
        return walk_from_arithmetic_root(interp, scope, node->left) - walk_from_arithmetic_root(interp, scope, node->right);
    } else if (node->op->type == Token::Type::OP_MULTIPLY) {
        return walk_from_arithmetic_root(interp, scope, node->left) * walk_from_arithmetic_root(interp, scope, node->right);
    } else {
        return walk_from_arithmetic_root(interp, scope, node->left) / walk_from_arithmetic_root(interp, scope, node->right);
    }
}

float walk_number_node(Scope *scope, Number_Node *node) {
    return node->value;
}

float walk_from_arithmetic_root(Interpreter *interp, Scope *scope, Ast_Node *node) {
    if (node->node_type == Ast_Node::Type::BINARY_OP) {
        return walk_binary_op_node(interp, scope, static_cast<Binary_Op_Node *>(node));
    } else if (node->node_type == Ast_Node::Type::UNARY_OP) {
        return walk_unary_op_node(interp, scope, static_cast<Unary_Op_Node *>(node));
    } else if (node->node_type == Ast_Node::Type::NUMBER) {
        return walk_number_node(scope, static_cast<Number_Node *>(node));
    } else {
        return walk_num_variable_node(interp, scope, static_cast<Variable_Node *>(node));
    }
}

float walk_num_function_node(Interpreter *interp, Scope *scope, Function_Definition_Node *def) {
    return 0;
}

std::string walk_str_function_node(Interpreter *interp, Scope *scope, Function_Definition_Node *def) {
    return "";
}

void walk_block_node(Interpreter *interp, Scope *scope, Block_Node *root) {
    Scope *block_scope = new Scope(scope);

    for (Ast_Node *child : root->children) {
        walk_from_root(interp, block_scope, child);
    }
}

float walk_num_variable_node(Interpreter *interp, Scope *scope, Variable_Node *node) {
    std::string name = node->name;
    Var_With_Success *var = get_var(scope, name);

    if (var->was_success) {
        // Variables stored as maps of string name to string value so need to convert to float
        return var->num_value;
    } else {
        std::cerr << "Use of unassigned num variable '" << name << "' at line: " << node->token->line_number << std::endl;
        return 0;
    }
}

std::string walk_str_variable_node(Interpreter *interp, Scope *scope, Variable_Node *node) {
    std::string name = node->name;
    Var_With_Success *var = get_var(scope, name);

    if (var->was_success) {
        return var->str_value;
    } else {
        std::cerr << "Use of unassigned str variable '" << name << "' at line: " << node->token->line_number << std::endl;
        return 0;
    }
}

void add_function_def_to_scope(Interpreter *interp, Scope *scope, Function_Definition_Node *def) {
    set_func(scope, def->name, def);
}

void walk_function_call(Interpreter *interp, Scope *scope, Function_Call_Node *call) {
    Func_With_Success *fs = get_func(scope, call->name);

    if (fs->was_success) {
        walk_block_node(interp, scope, fs->body->block);
    } else {
        std::cerr << "Attempted to call function: '" << call->name << "' without first defining it" << std::endl;
    }
}

void walk_assignment_node(Interpreter *interp, Scope *scope, Assignment_Node *node) {
    std::string name = node->left->name;
    Ast_Node::Type type = node->right->node_type;

    if (node->right->node_type == Ast_Node::Type::STRING) {
        // @ROBUSTNESS(HIGH) Handle str b = a;
        set_var(scope, name, static_cast<String_Node *>(node->right)->value);
    } else {
        set_var(scope, name, std::to_string(walk_from_arithmetic_root(interp, scope, node->right)));
    }
}

void walk_from_root(Interpreter *interp, Scope *scope, Ast_Node *root) {
    if (root->node_type == Ast_Node::Type::BLOCK) {
        walk_block_node(interp, scope, static_cast<Block_Node *>(root));
    } else if (root->node_type == Ast_Node::Type::FUNCTION_DEFINITION) {
        add_function_def_to_scope(interp, scope, static_cast<Function_Definition_Node *>(root));
    } else if (root->node_type == Ast_Node::Type::FUNCTION_CALL) {
        walk_function_call(interp, scope, static_cast<Function_Call_Node *>(root));
    } else if (root->node_type == Ast_Node::Type::ASSIGNMENT) {
        walk_assignment_node(interp, scope, static_cast<Assignment_Node *>(root));
    }
}

void interpret(Interpreter *interp) {
    Block_Node *root = parse(interp->parser);
    walk_from_root(interp, nullptr, root);
}

#include <iostream>
#include <sstream>
#include "interp.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "scope.hpp"
#include "shel_lib.hpp"

float walk_from_arithmetic_root(Interpreter *interp, Scope *scope, Ast_Node *node);
float get_number_variable(Interpreter *interp, Scope *scope, Ast_Variable *node);
void walk_from_root(Interpreter *interp, Scope *scope,  Ast_Node *root);
void call_native_function(Interpreter *interp, Scope *scope, Ast_Function_Call *call);

float walk_unary_op_node(Interpreter *interp, Scope *scope, Ast_Unary_Op *node) {
    Token::Type type = node->op->type;

    if (type == Token::Type::OP_PLUS) {
        return +walk_from_arithmetic_root(interp, scope, node->node);
    } else {
        return -walk_from_arithmetic_root(interp, scope, node->node);
    }
}

float walk_binary_op_node(Interpreter *interp, Scope *scope, Ast_Binary_Op *node) {
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

float walk_from_arithmetic_root(Interpreter *interp, Scope *scope, Ast_Node *node) {
    if (node->node_type == Ast_Node::Type::BINARY_OP) {
        return walk_binary_op_node(interp, scope, static_cast<Ast_Binary_Op *>(node));
    } else if (node->node_type == Ast_Node::Type::UNARY_OP) {
        return walk_unary_op_node(interp, scope, static_cast<Ast_Unary_Op *>(node));
    } else if (node->node_type == Ast_Node::Type::LITERAL) {
        return get_number_literal(scope, static_cast<Ast_Literal *>(node));
    } else if (node->node_type == Ast_Node::Type::FUNCTION_CALL) {
        return walk_function_call(interp, scope, static_cast<Ast_Function_Call *>(node));
    } else {
        return get_number_variable(interp, scope, static_cast<Ast_Variable *>(node));
    }
}

float get_number_literal(Scope *scope, Ast_Literal *node) {
    return std::stof(node->value);
}

std::string get_string_literal(Scope *scope, Ast_Literal *node) {
    return node->value;
}

float get_number_variable(Interpreter *interp, Scope *scope, Ast_Variable *node) {
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

float walk_block_node(Interpreter *interp, Scope *scope, Ast_Block *root) {
    for (Ast_Node *child : root->children) {
        walk_from_root(interp, scope, child);
    }

    if (root->return_node->value == NULL) return 0; // Empty block
    return walk_from_arithmetic_root(interp, scope, root->return_node->value);
}

std::string get_string_variable(Interpreter *interp, Scope *scope, Ast_Variable *node) {
    std::string name = node->name;
    Var_With_Success *var = get_var(scope, name);

    if (var->was_success) {
        return var->str_value;
    } else {
        std::cerr << "Use of unassigned str variable '" << name << "' at line: " << node->token->line_number << std::endl;
        return 0;
    }
}

void add_function_def_to_scope(Interpreter *interp, Scope *scope, Ast_Function_Definition *def) {
    set_func(scope, def->name, def);
}

float walk_function_call(Interpreter *interp, Scope *scope, Ast_Function_Call *call) {
    Func_With_Success *fs = get_func(scope, call->name);

    if (fs->was_success) {
        Scope *func_scope = new Scope(scope);

        // Match calculated values to function names and insert them into the function scope
        // before we walk the main function block
        for (int i = 0; i < fs->body->args.size(); i++) {
            Ast_Function_Argument *current = fs->body->args[i];

            set_var(func_scope, current->name, std::to_string(walk_from_arithmetic_root(interp, scope, call->args[i])));
        }

        return walk_block_node(interp, func_scope, fs->body->block);
    } else {
        call_native_function(interp, scope, call);
        return 0;
    }
}

bool is_num(std::string str) {
    std::istringstream iss(str);
    float f;
    iss >> std::noskipws >> f;

    return iss.eof() && iss.fail() == false;
}

void walk_assignment_node(Interpreter *interp, Scope *scope, Ast_Assignment *node) {
    std::string name = node->left->name;
    Ast_Node::Type type = node->right->node_type;

    if (type == Ast_Node::Type::LITERAL) {
        Ast_Literal *lit = static_cast<Ast_Literal *>(node->right);

        if (is_num(lit->value)) {
            set_var(scope, name, std::to_string(walk_from_arithmetic_root(interp, scope, node->right)));
        } else {
            set_var(scope, name, lit->value);
        }
    } else {
        set_var(scope, name, std::to_string(walk_from_arithmetic_root(interp, scope, node->right)));
    }
}

void walk_from_root(Interpreter *interp, Scope *scope, Ast_Node *root) {
    if (root->node_type == Ast_Node::Type::BLOCK) {
        walk_block_node(interp, scope, static_cast<Ast_Block *>(root));
    } else if (root->node_type == Ast_Node::Type::FUNCTION_DEFINITION) {
        add_function_def_to_scope(interp, scope, static_cast<Ast_Function_Definition *>(root));
    } else if (root->node_type == Ast_Node::Type::FUNCTION_CALL) {
        walk_function_call(interp, scope, static_cast<Ast_Function_Call *>(root));
    } else if (root->node_type == Ast_Node::Type::ASSIGNMENT) {
        walk_assignment_node(interp, scope, static_cast<Ast_Assignment *>(root));
    }
}

void call_native_function(Interpreter *interp, Scope *scope, Ast_Function_Call *call) {
    // @HACK(HIGH) @ROBUSTNESS(HIGH) Need better way of calling out to native functions
    // Just in here so it's easy to print out variable values in a .shel file at the min
    std::vector<float> evaluated_args;

    for (Ast_Node *arg : call->args) {
        float evaluated_arg = walk_from_arithmetic_root(interp, scope, arg);

        evaluated_args.push_back(evaluated_arg);
    }

    if (call->name == "print") {
        print(std::to_string(evaluated_args[0]));
    } else {
        std::cerr << "Attempted to call function: '" << call->name << "' without first defining it" << std::endl;
    }
}

void interpret(Interpreter *interp) {
    Scope *global_scope = new Scope(nullptr);

    Ast_Block *root = parse(interp->parser);
    walk_from_root(interp, global_scope, root);
}


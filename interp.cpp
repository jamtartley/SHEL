#include <iostream>
#include <sstream>

#include "interp.hpp"
#include "lexer.hpp"
#include "logger.hpp"
#include "parser.hpp"
#include "scope.hpp"
#include "shel_lib.hpp"

float walk_from_arithmetic_root(Interpreter *interp, Scope *scope, Ast_Node *node);
float get_number_variable(Interpreter *interp, Scope *scope, Ast_Variable *node);
bool is_num(std::string str);
Return_Value *walk_from_root(Interpreter *interp, Scope *scope, Ast_Node *root);

std::string get_unassigned_variable_error(std::string name, int line_number) {
    std::stringstream ss;
    ss << "Use of unassigned num variable '" << name << "' at line: " << line_number << std::endl;

    return ss.str();
}

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
        return walk_function_call(interp, scope, static_cast<Ast_Function_Call *>(node))->num_val;
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
        report_fatal_error(get_unassigned_variable_error(name, node->token->line_number));
        return 0;
    }
}

Return_Value *walk_block_node(Interpreter *interp, Scope *scope, Ast_Block *root) {
    for (Ast_Node *child : root->children) {
        Return_Value *ret = walk_from_root(interp, scope, child);
        if (ret != NULL) return ret;
    }

    return nullptr;
}

std::string get_string_variable(Interpreter *interp, Scope *scope, Ast_Variable *node) {
    std::string name = node->name;
    Var_With_Success *var = get_var(scope, name);

    if (var->was_success) {
        return var->str_value;
    } else {
        report_fatal_error(get_unassigned_variable_error(name, node->token->line_number));
        return 0;
    }
}

bool evaluate_comparison(Interpreter *interp, Scope *scope, Ast_Comparison *comparison) {
    // @TODO(HIGH) Don't assume numbers in comparison
    float left = walk_from_arithmetic_root(interp, scope, comparison->left);
    float right = walk_from_arithmetic_root(interp, scope, comparison->right);

    switch (comparison->comparator->type) {
        default:
        case Token::Type::COMPARE_EQUALS:
            return left == right;
        case Token::Type::COMPARE_NOT_EQUALS:
            return left != right;
        case Token::Type::COMPARE_GREATER_THAN:
            return left > right;
        case Token::Type::COMPARE_GREATER_THAN_EQUALS:
            return left >= right;
        case Token::Type::COMPARE_LESS_THAN:
            return left < right;
        case Token::Type::COMPARE_LESS_THAN_EQUALS:
            return left <= right;
    }
}

Return_Value *walk_if(Interpreter *interp, Scope *scope, Ast_If *if_node) {
    // @TODO(HIGH) Allow 'else if' branching in if statement
    bool is_success = evaluate_comparison(interp, scope, if_node->comparison);

    if (is_success) {
        return walk_block_node(interp, new Scope(scope), if_node->success);
    } else {
        if (if_node->failure != NULL) {
            return walk_block_node(interp, new Scope(scope), if_node->failure);
        }
    }

    return nullptr;
}

Return_Value *walk_while(Interpreter *interp, Scope *scope, Ast_While *while_node) {
    Return_Value *ret = NULL;

    while (evaluate_comparison(interp, scope, while_node->comparison)) {
        ret = walk_block_node(interp, new Scope(scope), while_node->body);

        if (ret != NULL) break;
    }

    return ret;
}

Return_Value *walk_loop(Interpreter *interp, Scope *scope, Ast_Loop *loop_node) {
    // @ROBUSTNESS(MEDIUM) Check for weird from loop setups
    // i.e. positive step value but higher start value than end
    float from = walk_from_arithmetic_root(interp, scope, loop_node->start);
    float to = walk_from_arithmetic_root(interp, scope, loop_node->to);
    float step = walk_from_arithmetic_root(interp, scope, loop_node->step);
    Return_Value *ret = NULL;
    bool is_going_up = to > from;

    for (float i = from; is_going_up ? i <= to : i >= to; i += step) {
        Scope *body_scope = new Scope(scope);
        assign_var(body_scope, "idx", std::to_string(i));

        ret = walk_block_node(interp, body_scope, loop_node->body);

        if (ret != NULL) break;
    }

    return ret;
}

void add_function_def_to_scope(Interpreter *interp, Scope *scope, Ast_Function_Definition *def) {
    set_func(scope, def->name, def);
}

Return_Value *walk_function_call(Interpreter *interp, Scope *scope, Ast_Function_Call *call) {
    Func_With_Success *fs = get_func(scope, call->name);

    if (fs->was_success) {
        Scope *func_scope = new Scope(scope);

        // Match calculated values to function names and insert them into the function scope
        // before we walk the main function block
        for (int i = 0; i < fs->body->args.size(); i++) {
            Ast_Function_Argument *current = fs->body->args[i];
            std::string val = std::to_string(walk_from_arithmetic_root(interp, scope, call->args[i]));
            assign_var(func_scope, current->name, val);
        }

        return walk_block_node(interp, func_scope, fs->body->block);
    } else {
        // @HACK(HIGH) Horrible call out to native print
        if (call->name == "print") {
            // This just isn't very nice
            std::string main_string = static_cast<Ast_Literal *>(call->args[0])->value;
            std::vector<std::string> args;

            for (int i = 1; i < call->args.size(); i++) {
                args.push_back(std::to_string(walk_from_arithmetic_root(interp, scope, call->args[i])));
            }

            print_native(main_string, args);
        }
        return nullptr;
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
    std::string value;

    if (type == Ast_Node::Type::LITERAL) {
        Ast_Literal *lit = static_cast<Ast_Literal *>(node->right);

        if (is_num(lit->value)) {
            value = std::to_string(walk_from_arithmetic_root(interp, scope, node->right));
        } else {
            value = lit->value;
        }
    } else {
        value = std::to_string(walk_from_arithmetic_root(interp, scope, node->right));
    }

    if (node->is_first_assign) {
        assign_var(scope, name, value);
    } else {
        reassign_var(scope, name, value);
    }
}

Return_Value *walk_from_root(Interpreter *interp, Scope *scope, Ast_Node *root) {
    if (root->node_type == Ast_Node::Type::BLOCK) {
        return walk_block_node(interp, scope, static_cast<Ast_Block *>(root));
    } else if (root->node_type == Ast_Node::Type::FUNCTION_DEFINITION) {
        add_function_def_to_scope(interp, scope, static_cast<Ast_Function_Definition *>(root));
        return nullptr;
    } else if (root->node_type == Ast_Node::Type::RETURN) {
        float d = walk_from_arithmetic_root(interp, scope, static_cast<Ast_Return *>(root)->value);
        return new Return_Value(d);
    } else if (root->node_type == Ast_Node::Type::IF) {
        return walk_if(interp, scope, static_cast<Ast_If *>(root));
    } else if (root->node_type == Ast_Node::Type::WHILE) {
        return walk_while(interp, scope, static_cast<Ast_While *>(root));
    } else if (root->node_type == Ast_Node::Type::LOOP) {
        return walk_loop(interp, scope, static_cast<Ast_Loop *>(root));
    } else if (root->node_type == Ast_Node::Type::FUNCTION_CALL) {
        return walk_function_call(interp, scope, static_cast<Ast_Function_Call *>(root));
    } else if (root->node_type == Ast_Node::Type::ASSIGNMENT) {
        walk_assignment_node(interp, scope, static_cast<Ast_Assignment *>(root));
        return nullptr;
    } else {
        return nullptr;
    }
}

void interpret(Interpreter *interp) {
    Scope *global_scope = new Scope(nullptr);

    Ast_Block *root = parse(interp->parser);
    walk_from_root(interp, global_scope, root);
}


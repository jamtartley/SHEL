#include <iostream>
#include <sstream>

#include "interp.hpp"
#include "lexer.hpp"
#include "logger.hpp"
#include "parser.hpp"
#include "scope.hpp"
#include "shel_lib.hpp"

Data_Value *walk_from_root(Interpreter *interp, Scope *scope, Ast_Node *root);

std::string get_unassigned_variable_error(std::string name, int line_number) {
    std::stringstream ss;
    ss << "Use of unassigned num variable '" << name << "' at line: " << line_number << std::endl;

    return ss.str();
}

Data_Value *walk_expression(Interpreter *interp, Scope *scope, Ast_Node *node) {
    if (node->node_type == Ast_Node::Type::BINARY_OP) {
        return walk_binary_op_node(interp, scope, (Ast_Binary_Op *)node);
    } else if (node->node_type == Ast_Node::Type::UNARY_OP) {
        return walk_unary_op_node(interp, scope, (Ast_Unary_Op *)node);
    } else if (node->node_type == Ast_Node::Type::LITERAL) {
        return get_data_from_literal(interp, scope, (Ast_Literal *)node);
    } else if (node->node_type == Ast_Node::Type::FUNCTION_CALL) {
        return walk_function_call(interp, scope, (Ast_Function_Call *)node);
    } else if (node->node_type == Ast_Node::Type::VARIABLE) {
        return get_variable(interp, scope, (Ast_Variable *)node);
    } else {
        return nullptr;
    }
}

Data_Value *walk_unary_op_node(Interpreter *interp, Scope *scope, Ast_Unary_Op *node) {
    Token::Type type = node->op->type;
    Data_Value *value = walk_expression(interp, scope, node->node);

    if (value->data_type != Data_Type::NUM) {
        report_fatal_error("Attempted to perform unary operation on non-num type");
        return nullptr;
    } else {
        return new Data_Value(type == Token::Type::OP_PLUS ? +value->num_val : -value->num_val);
    }
}

Data_Value *walk_binary_op_node(Interpreter *interp, Scope *scope, Ast_Binary_Op *node) {
    Data_Value *left = walk_expression(interp, scope, node->left);
    Data_Value *right = walk_expression(interp, scope, node->right);

    fail_if_binary_op_invalid(left, right, node->op);

    if (node->op->type == Token::Type::OP_PLUS) {
        if (left->data_type == Data_Type::NUM) {
            return new Data_Value(left->num_val + right->num_val);
        } else if (left->data_type == Data_Type::STR) {
            return new Data_Value(left->str_val + right->str_val);
        } else {
            report_fatal_error("Attempted to '+' incompatible expressions");
            return nullptr;
        }
        return new Data_Value(left->num_val + right->num_val);
    } else if (node->op->type == Token::Type::OP_MINUS) {
        return new Data_Value(left->num_val - right->num_val);
    } else if (node->op->type == Token::Type::OP_MULTIPLY) {
        return new Data_Value(left->num_val * right->num_val);
    } else if (node->op->type == Token::Type::OP_DIVIDE) {
        return new Data_Value(left->num_val / right->num_val);
    } else if (node->op->type == Token::Type::OP_MODULO) {
        return new Data_Value(float(int(left->num_val) % int(right->num_val)));
    } else {
        if (node->op->flags & Token::Flags::COMPARISON || node->op->flags & Token::Flags::LOGICAL) {
            return new Data_Value(evaluate_node_to_bool(interp, scope, node));
        }

        return nullptr;
    }
}

void fail_if_binary_op_invalid(Data_Value *left, Data_Value *right, Token *op) {
    Data_Type left_t = left->data_type;
    Data_Type right_t = right->data_type;

    if (left_t != right_t) {
        report_fatal_error("Cannot perform binary operations on two mismatched expression types");
    }

    switch (op->type) {
        case Token::Type::OP_PLUS:
        case Token::Type::COMPARE_EQUALS:
        case Token::Type::COMPARE_NOT_EQUALS:
            // Can + both num and str
            return;
        case Token::Type::COMPARE_LOGICAL_AND:
        case Token::Type::COMPARE_LOGICAL_OR:
            if (left_t != Data_Type::BOOL) {
                report_fatal_error("Cannot perform logical operations on non bool expressions");
            }
            return;
        case Token::Type::OP_MINUS:
        case Token::Type::OP_MULTIPLY:
        case Token::Type::OP_DIVIDE:
        case Token::Type::OP_MODULO:
        case Token::Type::COMPARE_GREATER_THAN:
        case Token::Type::COMPARE_LESS_THAN:
        case Token::Type::COMPARE_GREATER_THAN_EQUALS:
        case Token::Type::COMPARE_LESS_THAN_EQUALS: {
            if (left_t == Data_Type::STR || left_t == Data_Type::BOOL) {
                std::stringstream ss;
                ss << "Cannot perform '" << op->value << "' on two string types";
                report_fatal_error(ss.str());
            }
            return;
        }
        default:
            report_fatal_error("Invalid binary operator");
            return;
    }
}

Data_Value *get_variable(Interpreter *interp, Scope *scope, Ast_Variable *node) {
    std::string name = node->name;
    Var_With_Success *var = get_var(scope, name);

    if (var->was_success) {
        // Variables stored as maps of string name to string value so need to convert to float
        return var->data;
    } else {
        report_fatal_error(get_unassigned_variable_error(name, node->token->line_number));
        return nullptr;
    }
}

Data_Value *get_data_from_literal(Interpreter *interp, Scope *scope, Ast_Literal *lit) {
    switch (lit->data_type) {
        case Data_Type::NUM: return new Data_Value(std::stof(lit->value));
        case Data_Type::STR: return new Data_Value(lit->value);
        case Data_Type::BOOL: return new Data_Value(lit->value == "true" ? true : false);
        default: return nullptr;
    }
}

Data_Value *walk_block_node(Interpreter *interp, Scope *scope, Ast_Block *root) {
    for (Ast_Node *child : root->children) {
        Data_Value *ret = walk_from_root(interp, scope, child);
        if (ret != NULL) return ret;
    }

    return nullptr;
}

bool evaluate_binary_op_to_bool(Interpreter *interp, Scope *scope, Ast_Binary_Op *comparison) {
    Data_Value *left = walk_expression(interp, scope, comparison->left);
    Data_Value *right = walk_expression(interp, scope, comparison->right);

    if (left->data_type != right->data_type) {
        report_fatal_error("Attempted to compare expressions of different data types");
    }

    Data_Type type = left->data_type;

    switch (comparison->op->type) {
        default:
        case Token::Type::COMPARE_EQUALS:
            if (type == Data_Type::NUM) return left->num_val == right->num_val;
            if (type == Data_Type::STR) return left->str_val == right->str_val;
            if (type == Data_Type::BOOL) return left->bool_val == right->bool_val;
            return false;
        case Token::Type::COMPARE_NOT_EQUALS:
            if (type == Data_Type::NUM) return left->num_val != right->num_val;
            if (type == Data_Type::STR) return left->str_val != right->str_val;
            if (type == Data_Type::BOOL) return left->bool_val != right->bool_val;
            return false;
        case Token::Type::COMPARE_GREATER_THAN:
            if (type == Data_Type::NUM) return left->num_val > right->num_val;
            report_fatal_error("Invalid comparison operator for given data type");
            return false;
        case Token::Type::COMPARE_GREATER_THAN_EQUALS:
            if (type == Data_Type::NUM) return left->num_val >= right->num_val;
            report_fatal_error("Invalid comparison operator for given data type");
            return false;
        case Token::Type::COMPARE_LESS_THAN:
            if (type == Data_Type::NUM) return left->num_val < right->num_val;
            report_fatal_error("Invalid comparison operator for given data type");
            return false;
        case Token::Type::COMPARE_LESS_THAN_EQUALS:
            if (type == Data_Type::NUM) return left->num_val <= right->num_val;
            report_fatal_error("Invalid comparison operator for given data type");
            return false;
        case Token::Type::COMPARE_LOGICAL_OR:
            if (type == Data_Type::BOOL) return left->bool_val || right->bool_val;
            report_fatal_error("Invalid comparison operator for given data type");
            return false;
        case Token::Type::COMPARE_LOGICAL_AND:
            if (type == Data_Type::BOOL) return left->bool_val && right->bool_val;
            report_fatal_error("Invalid comparison operator for given data type");
            return false;
    }
}

bool evaluate_node_to_bool(Interpreter *interp, Scope *scope, Ast_Node *node) {
    if (node->node_type == Ast_Node::Type::BINARY_OP) {
        return evaluate_binary_op_to_bool(interp, scope, (Ast_Binary_Op *)node);
    } else if (node->node_type == Ast_Node::Type::LITERAL) {
        Ast_Literal *lit = (Ast_Literal *)node;
        if (lit->data_type == Data_Type::BOOL) return lit->value == "true";
    } else if (node->node_type == Ast_Node::Type::VARIABLE) {
        Data_Value *value = get_variable(interp, scope, (Ast_Variable *)node);
        if (value->data_type == Data_Type::BOOL) return value->bool_val;
    } else if (node->node_type == Ast_Node::Type::BLOCK) {
        Data_Value *value = walk_block_node(interp, scope, (Ast_Block *)node);
        if (value->data_type == Data_Type::BOOL) return value->bool_val;
    } else if (node->node_type == Ast_Node::Type::FUNCTION_CALL) {
        Data_Value *value = walk_function_call(interp, scope, (Ast_Function_Call *)node);
        if (value->data_type == Data_Type::BOOL) return value->bool_val;
    }

    report_fatal_error("Invalid comparison");
    return false;
}

Data_Value *walk_if(Interpreter *interp, Scope *scope, Ast_If *if_node) {
    // @TODO(HIGH) Allow 'else if' branching in if statement
    bool is_success = evaluate_node_to_bool(interp, scope, if_node->comparison);

    if (is_success) {
        return walk_block_node(interp, new Scope(scope), if_node->success);
    } else {
        if (if_node->failure != NULL) {
            return walk_block_node(interp, new Scope(scope), if_node->failure);
        }
    }

    return nullptr;
}

Data_Value *walk_while(Interpreter *interp, Scope *scope, Ast_While *while_node) {
    Data_Value *ret = NULL;

    while (evaluate_node_to_bool(interp, scope, while_node->comparison)) {
        ret = walk_block_node(interp, new Scope(scope), while_node->body);

        if (ret != NULL) break;
    }

    return ret;
}

Data_Value *walk_loop(Interpreter *interp, Scope *scope, Ast_Loop *loop_node) {
    Data_Value *from = walk_expression(interp, scope, loop_node->start);
    Data_Value *to = walk_expression(interp, scope, loop_node->to);
    Data_Value *step = walk_expression(interp, scope, loop_node->step);

    if (from->data_type != Data_Type::NUM || to->data_type != Data_Type::NUM || step->data_type != Data_Type::NUM) {
        report_fatal_error("Attempted to use non-num expression as control in a from loop");
    }

    if (step->num_val < 0 && from->num_val < to->num_val) {
        report_fatal_error("from < to but step value is negative");
    }

    if (step->num_val > 0 && from->num_val > to->num_val) {
        report_fatal_error("to > from but step value is positive");
    }

    if (step->num_val == 0) {
        report_fatal_error("step value cannot be 0");
    }

    Data_Value *ret = NULL;
    bool is_going_up = to->num_val > from->num_val;

    for (float i = from->num_val; is_going_up ? i <= to->num_val : i >= to->num_val; i += step->num_val) {
        Scope *body_scope = new Scope(scope);
        assign_var(body_scope, "idx", new Data_Value(float(i)));

        ret = walk_block_node(interp, body_scope, loop_node->body);

        if (ret != NULL) break;
    }

    return ret;
}

void add_function_def_to_scope(Interpreter *interp, Scope *scope, Ast_Function_Definition *def) {
    set_func(scope, def->name, def);
}

std::string get_string_from_return_value(Data_Value *ret) {
    if (ret->data_type == Data_Type::NUM) return std::to_string(ret->num_val);
    if (ret->data_type == Data_Type::STR) return ret->str_val;
    if (ret->data_type == Data_Type::BOOL) return std::to_string(ret->bool_val);

    report_fatal_error("");
    return "";
}

Data_Value *walk_function_call(Interpreter *interp, Scope *scope, Ast_Function_Call *call) {
    Func_With_Success *fs = get_func(scope, call->name);

    if (fs->was_success) {
        Scope *func_scope = new Scope(scope);

        // Match calculated values to function names and insert them into the function scope
        // before we walk the main function block
        for (int i = 0; i < fs->body->args.size(); i++) {
            Ast_Function_Argument *current = fs->body->args[i];
            Data_Value *expr = walk_expression(interp, scope, call->args[i]);

            assign_var(func_scope, current->name, expr);
        }

        return walk_block_node(interp, func_scope, fs->body->block);
    } else {
        // @HACK(HIGH) Horrible call out to native print
        if (call->name == "print") {
            // This just isn't very nice
            std::string main_string = ((Ast_Literal *)call->args[0])->value;
            std::vector<std::string> args;

            for (int i = 1; i < call->args.size(); i++) {
                args.push_back(get_string_from_return_value(walk_expression(interp, scope, call->args[i])));
            }

            print_native(main_string, args);
        }
        return nullptr;
    }
}

void walk_assignment_node(Interpreter *interp, Scope *scope, Ast_Assignment *node) {
    std::string name = node->left->name;
    Ast_Node::Type type = node->right->node_type;
    Data_Value *expr = walk_expression(interp, scope, node->right);

    if (node->is_first_assign) {
        assign_var(scope, name, expr);
    } else {
        reassign_var(scope, name, expr);
    }
}

Data_Value *walk_from_root(Interpreter *interp, Scope *scope, Ast_Node *root) {
    if (root->node_type == Ast_Node::Type::BLOCK) {
        return walk_block_node(interp, scope, (Ast_Block *)root);
    } else if (root->node_type == Ast_Node::Type::FUNCTION_DEFINITION) {
        add_function_def_to_scope(interp, scope, (Ast_Function_Definition *)root);
        return nullptr;
    } else if (root->node_type == Ast_Node::Type::RETURN) {
        return walk_expression(interp, scope, ((Ast_Return *)root)->value);
    } else if (root->node_type == Ast_Node::Type::IF) {
        return walk_if(interp, scope, (Ast_If *)root);
    } else if (root->node_type == Ast_Node::Type::WHILE) {
        return walk_while(interp, scope, (Ast_While *)root);
    } else if (root->node_type == Ast_Node::Type::LOOP) {
        return walk_loop(interp, scope, (Ast_Loop *)root);
    } else if (root->node_type == Ast_Node::Type::FUNCTION_CALL) {
        return walk_function_call(interp, scope, (Ast_Function_Call *)root);
    } else if (root->node_type == Ast_Node::Type::ASSIGNMENT) {
        walk_assignment_node(interp, scope, (Ast_Assignment *)root);
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


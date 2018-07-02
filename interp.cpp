#include <iostream>
#include <sstream>

#include "interp.hpp"
#include "lexer.hpp"
#include "logger.hpp"
#include "parser.hpp"
#include "scope.hpp"
#include "shel_lib.hpp"

std::string get_unassigned_variable_error(std::string name) {
    std::stringstream ss;
    ss << "Use of unassigned variable '" << name << "'";

    return ss.str();
}

void fail_if_binary_op_invalid(Data_Atom *left, Data_Atom *right, Token *op) {
    Data_Type left_t = left->data_type;
    Data_Type right_t = right->data_type;

    if (left_t != right_t) {
        report_fatal_error("Cannot perform binary operations on two mismatched expression types", op->site);
    }

    switch (op->type) {
        case Token::Type::OP_PLUS:
        case Token::Type::COMPARE_EQUALS:
        case Token::Type::COMPARE_NOT_EQUALS:
            // Can + both num and str
            return;
        case Token::Type::LOGICAL_AND:
        case Token::Type::LOGICAL_OR:
            if (left_t != Data_Type::BOOL) {
                report_fatal_error("Cannot perform logical operations on non bool expressions", op->site);
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
                report_fatal_error(ss.str(), op->site);
            }
            return;
        }
        default:
            report_fatal_error("Invalid binary operator", op->site);
            return;
    }
}

std::string get_string_from_return_value(Data_Atom *ret) {
    if (ret->data_type == Data_Type::NUM) return std::to_string(((Num_Atom *)ret)->value);
    if (ret->data_type == Data_Type::STR) return ((Str_Atom *)ret)->value;
    if (ret->data_type == Data_Type::BOOL) return std::to_string(((Bool_Atom *)ret)->value);

    report_fatal_error("");
    return "";
}

Data_Atom *Interpreter::walk_block_node(Scope *scope, Ast_Block *root) {
    // @TODO(HIGH) Clean up return weirdness
    // Return doesn't really work as you would expect coming from most languages
    // at the minute in that it only returns from the immediate block. It should
    // really bubble up until it finds a a function definition or leaves global scope.
    for (Ast_Node *child : root->children) {
        Data_Atom *ret = walk_from_root(scope, child);
        if (ret != NULL) return ret;
    }

    return NULL;
}

Data_Atom *Interpreter::walk_expression(Scope *scope, Ast_Node *node) {
    if (node->node_type == Ast_Node::Type::BINARY_OP) {
        return walk_binary_op_node(scope, (Ast_Binary_Op *)node);
    } else if (node->node_type == Ast_Node::Type::UNARY_OP) {
        return walk_unary_op_node(scope, (Ast_Unary_Op *)node);
    } else if (node->node_type == Ast_Node::Type::LITERAL) {
        return get_data_from_literal(scope, (Ast_Literal *)node);
    } else if (node->node_type == Ast_Node::Type::ARRAY) {
        return walk_array_node(scope, (Ast_Array *)node);
    } else if (node->node_type == Ast_Node::Type::FUNCTION_CALL) {
        return walk_function_call(scope, (Ast_Function_Call *)node);
    } else if (node->node_type == Ast_Node::Type::VARIABLE) {
        return get_variable(scope, (Ast_Variable *)node);
    } else {
        return NULL;
    }
}

Data_Atom *Interpreter::walk_binary_op_node(Scope *scope, Ast_Binary_Op *node) {
    Data_Atom *left = walk_expression(scope, node->left);
    Data_Atom *right = walk_expression(scope, node->right);

    fail_if_binary_op_invalid(left, right, node->op);

    if (node->op->type == Token::Type::OP_PLUS) {
        if (left->data_type == Data_Type::NUM) {
            return new Num_Atom(((Num_Atom *)left)->value + ((Num_Atom *)right)->value);
        } else if (left->data_type == Data_Type::STR) {
            return new Str_Atom(((Str_Atom *)left)->value + ((Str_Atom *)right)->value);
        } else {
            report_fatal_error("Attempted to '+' incompatible expressions", node->op->site);
            return NULL;
        }
    } else if (node->op->type == Token::Type::OP_MINUS) {
        return new Num_Atom(((Num_Atom *)left)->value - ((Num_Atom *)right)->value);
    } else if (node->op->type == Token::Type::OP_MULTIPLY) {
        return new Num_Atom(((Num_Atom *)left)->value * ((Num_Atom *)right)->value);
    } else if (node->op->type == Token::Type::OP_DIVIDE) {
        return new Num_Atom(((Num_Atom *)left)->value / ((Num_Atom *)right)->value);
    } else if (node->op->type == Token::Type::OP_MODULO) {
        Num_Atom *l = (Num_Atom *)left;
        Num_Atom *r = (Num_Atom *)right;
        return new Num_Atom(float(int(l->value) % int(r->value)));
    } else {
        if (node->op->flags & Token::Flags::COMPARISON || node->op->flags & Token::Flags::LOGICAL) {
            return new Bool_Atom(evaluate_node_to_bool(scope, node));
        }

        return NULL;
    }
}

Array_Atom *Interpreter::walk_array_node(Scope *scope, Ast_Array *array) {
    std::vector<Data_Atom *> items;

    for (Ast_Node *item : array->items) {
        items.push_back(walk_expression(scope, item));
    }

    Data_Type type = Data_Type::UNKNOWN;

    if (items.size() > 0) type = items[0]->data_type;

    return new Array_Atom(items, type);
}

Data_Atom *Interpreter::walk_unary_op_node(Scope *scope, Ast_Unary_Op *node) {
    Token::Type type = node->op->type;
    Data_Atom *atom = walk_expression(scope, node->node);

    if (atom->data_type == Data_Type::BOOL) {
        if (type == Token::Type::LOGICAL_NOT) {
            return (Bool_Atom *)atom;
        } else {
            report_fatal_error("Attempted invalid unary operation on bool value", node->site);
            return NULL;
        }
    }

    if (atom->data_type == Data_Type::NUM) {
        Num_Atom *num = (Num_Atom *)atom;
        if (type == Token::Type::OP_PLUS) {
            num->value = +num->value;
            return num;
        } else if (type == Token::Type::OP_MINUS) {
            num->value = -num->value;
            return num;
        } else {
            report_fatal_error("Attempted invalid unary operation on num value", node->op->site);
            return NULL;
        }
    }

    report_fatal_error("Attempted invalid unary operation on str value", node->op->site);
    return NULL;
}

Data_Atom *Interpreter::walk_function_call(Scope *scope, Ast_Function_Call *call) {
    Func_With_Success *fs = get_func(scope, call->name);

    if (fs->was_success) {
        auto *func_scope = new Scope(scope);
        auto *func_def = fs->func_def;

        if (call->args.size() != func_def->args.size()) {
            std::stringstream ss;
            ss << "Attempted to call '" << func_def->name << "' with an incorrect number of args. Expected " << func_def->args.size()
                << ", got " << call->args.size() << ".";
            report_fatal_error(ss.str(), call->args_start_site);
        }

        // Match calculated values to function names and insert them into the function scope
        // before we walk the main function block
        for (int i = 0; i < func_def->args.size(); i++) {
            Ast_Function_Argument *current = func_def->args[i];
            Data_Atom *expr = walk_expression(scope, call->args[i]);

            assign_var(func_scope, current->name, expr);
        }

        Data_Atom *block_return = walk_block_node(func_scope, func_def->block);

        // Programmer didn't write an explicit return statement, so we return void for them
        if (block_return == NULL) return NULL;

        if (block_return->data_type != func_def->data_type) {
            std::stringstream ss;
            ss << "Unexpected return type from function - wanted " << data_type_to_string(func_def->data_type)
                << ", but got " << data_type_to_string(block_return->data_type);
            report_fatal_error(ss.str(), func_def->block->return_node->site);
        }

        return block_return;
    } else {
        // @HACK(HIGH) Horrible call out to native print
        if (call->name == "print") {
            // This just isn't very nice
            std::string main_string = ((Ast_Literal *)call->args[0])->value;
            std::vector<std::string> args;

            for (int i = 1; i < call->args.size(); i++) {
                args.push_back(get_string_from_return_value(walk_expression(scope, call->args[i])));
            }

            print_native(main_string, args);

            return NULL;
        }

        std::stringstream ss;
        ss << "Attempted to call bug '" << call->name << "', which is either not in scope or does not exist";
        report_fatal_error(ss.str(), call->site);

        return NULL;
    }
}

Data_Atom *Interpreter::walk_if(Scope *scope, Ast_If *if_node) {
    while (if_node != NULL) {
        // Comparison is NULL in else node, so if we get there assume true
        if (if_node->comparison == NULL || evaluate_node_to_bool(scope, if_node->comparison)) {
            return walk_block_node(new Scope(scope), if_node->success);
        }

        if_node = if_node->failure;
    }

    return NULL;
}

Data_Atom *Interpreter::walk_while(Scope *scope, Ast_While *while_node) {
    Data_Atom *ret = NULL;

    while (evaluate_node_to_bool(scope, while_node->comparison)) {
        ret = walk_block_node(new Scope(scope), while_node->body);

        if (ret != NULL) break;
    }

    return ret;
}

Data_Atom *Interpreter::walk_loop(Scope *scope, Ast_Loop *loop_node) {
    Num_Atom *from = (Num_Atom *)walk_expression(scope, loop_node->start);
    Num_Atom *to = (Num_Atom *)walk_expression(scope, loop_node->to);
    Num_Atom *step = (Num_Atom *)walk_expression(scope, loop_node->step);

    if (from->data_type != Data_Type::NUM || to->data_type != Data_Type::NUM || step->data_type != Data_Type::NUM) {
        report_fatal_error("Attempted to use non-num expression as control in a from loop", loop_node->site);
    }

    if (step->value < 0 && from->value < to->value) {
        report_fatal_error("from < to but step value is negative", loop_node->site);
    }

    if (step->value > 0 && from->value > to->value) {
        report_fatal_error("to > from but step value is positive", loop_node->site);
    }

    if (step->value == 0) {
        report_fatal_error("step value cannot be 0", loop_node->site);
    }

    Data_Atom *ret = NULL;
    bool is_going_up = to->value > from->value;

    for (float i = from->value; is_going_up ? i <= to->value : i >= to->value; i += step->value) {
        auto *body_scope = new Scope(scope);
        assign_var(body_scope, "idx", new Num_Atom(float(i)));

        ret = walk_block_node(body_scope, loop_node->body);

        if (ret != NULL) break;
    }

    return ret;
}

Data_Atom *Interpreter::walk_from_root(Scope *scope, Ast_Node *root) {
    if (root->node_type == Ast_Node::Type::BLOCK) {
        return walk_block_node(scope, (Ast_Block *)root);
    } else if (root->node_type == Ast_Node::Type::FUNCTION_DEFINITION) {
        add_function_def_to_scope(scope, (Ast_Function_Definition *)root);
        return NULL;
    } else if (root->node_type == Ast_Node::Type::RETURN) {
        Data_Atom *val = walk_expression(scope, ((Ast_Return *)root)->value);
        if (val == NULL) return new Data_Atom(); // void return
        else return val;
    } else if (root->node_type == Ast_Node::Type::IF) {
        return walk_if(scope, (Ast_If *)root);
    } else if (root->node_type == Ast_Node::Type::WHILE) {
        return walk_while(scope, (Ast_While *)root);
    } else if (root->node_type == Ast_Node::Type::LOOP) {
        return walk_loop(scope, (Ast_Loop *)root);
    } else if (root->node_type == Ast_Node::Type::FUNCTION_CALL) {
        return walk_function_call(scope, (Ast_Function_Call *)root);
    } else if (root->node_type == Ast_Node::Type::ASSIGNMENT) {
        walk_assignment_node(scope, (Ast_Assignment *)root);
        return NULL;
    } else {
        return NULL;
    }
}

Data_Atom *Interpreter::get_variable(Scope *scope, Ast_Variable *node) {
    std::string name = node->name;
    Var_With_Success *var = get_var(scope, name);

    if (var->was_success) {
        // @ROBUSTNESS(HIGH) Getting array variable item
        if (var->data->data_type == Data_Type::ARRAY) {
            return ((Array_Atom *)var->data)->items.at(((Num_Atom *)walk_expression(scope, node->index))->value);
        } 
        return var->data;
    } else {
        report_fatal_error(get_unassigned_variable_error(name), node->token->site);
        return NULL;
    }
}

Data_Atom *Interpreter::get_data_from_literal(Scope *scope, Ast_Literal *lit) {
    switch (lit->data_type) {
        case Data_Type::NUM: return new Num_Atom(std::stof(lit->value));
        case Data_Type::STR: return new Str_Atom(lit->value);
        case Data_Type::BOOL: return new Bool_Atom(lit->value == "true" ? true : false);
        default: return NULL;
    }
}

bool Interpreter::evaluate_node_to_bool(Scope *scope, Ast_Node *node) {
    if (node->node_type == Ast_Node::Type::BINARY_OP) {
        return evaluate_binary_op_to_bool(scope, (Ast_Binary_Op *)node);
    } else if (node->node_type == Ast_Node::Type::LITERAL) {
        Ast_Literal *lit = (Ast_Literal *)node;
        if (lit->data_type == Data_Type::BOOL) return lit->value == "true";
    } else if (node->node_type == Ast_Node::Type::VARIABLE) {
        Data_Atom *atom = get_variable(scope, (Ast_Variable *)node);
        if (atom->data_type == Data_Type::BOOL) return ((Bool_Atom *)atom)->value;
    } else if (node->node_type == Ast_Node::Type::BLOCK) {
        Data_Atom *atom = walk_block_node(scope, (Ast_Block *)node);
        if (atom->data_type == Data_Type::BOOL) return ((Bool_Atom *)atom)->value;
    } else if (node->node_type == Ast_Node::Type::FUNCTION_CALL) {
        Data_Atom *atom = walk_function_call(scope, (Ast_Function_Call *)node);
        if (atom->data_type == Data_Type::BOOL) return ((Bool_Atom *)atom)->value;
    }

    report_fatal_error("Invalid comparison", node->site);
    return false;
}

bool Interpreter::evaluate_binary_op_to_bool(Scope *scope, Ast_Binary_Op *comparison) {
    Data_Atom *left = walk_expression(scope, comparison->left);
    Data_Atom *right = walk_expression(scope, comparison->right);

    if (left->data_type != right->data_type) {
        report_fatal_error("Attempted to compare expressions of different data types", comparison->site);
    }

    Data_Type type = left->data_type;

    switch (comparison->op->type) {
        default:
        case Token::Type::COMPARE_EQUALS:
            if (type == Data_Type::NUM)  return ((Num_Atom *)left)->value == ((Num_Atom *)right)->value;
            if (type == Data_Type::STR)  return ((Str_Atom *)left)->value == ((Str_Atom *)right)->value;
            if (type == Data_Type::BOOL) return ((Bool_Atom *)left)->value == ((Bool_Atom *)right)->value;
            break;
        case Token::Type::COMPARE_NOT_EQUALS:
            if (type == Data_Type::NUM)  return ((Num_Atom *)left)->value != ((Num_Atom *)right)->value;
            if (type == Data_Type::STR)  return ((Str_Atom *)left)->value != ((Str_Atom *)right)->value;
            if (type == Data_Type::BOOL) return ((Bool_Atom *)left)->value != ((Bool_Atom *)right)->value;
            break;
        case Token::Type::COMPARE_GREATER_THAN:
            if (type == Data_Type::NUM) return ((Num_Atom *)left)->value > ((Num_Atom *)right)->value;
            break;
        case Token::Type::COMPARE_GREATER_THAN_EQUALS:
            if (type == Data_Type::NUM) return ((Num_Atom *)left)->value >= ((Num_Atom *)right)->value;
            break;
        case Token::Type::COMPARE_LESS_THAN:
            if (type == Data_Type::NUM) return ((Num_Atom *)left)->value < ((Num_Atom *)right)->value;
            break;
        case Token::Type::COMPARE_LESS_THAN_EQUALS:
            if (type == Data_Type::NUM) return ((Num_Atom *)left)->value <= ((Num_Atom *)right)->value;
            break;
        case Token::Type::LOGICAL_OR:
            if (type == Data_Type::BOOL) return ((Bool_Atom *)left)->value || ((Bool_Atom *)right)->value;
            break;
        case Token::Type::LOGICAL_AND:
            if (type == Data_Type::BOOL) return ((Bool_Atom *)left)->value && ((Bool_Atom *)right)->value;
            break;
    }

    report_fatal_error("Invalid comparison operator for given data type", comparison->site);
    return false;
}

void Interpreter::walk_assignment_node(Scope *scope, Ast_Assignment *node) {
    std::string name = node->left->name;
    Data_Atom *expr = walk_expression(scope, node->right);

    if (node->is_first_assign) {
        auto left_data_type = node->left->data_type;
        auto right_data_type = expr->data_type == Data_Type::ARRAY ? ((Array_Atom *)expr)->atom_type : expr->data_type;

        if (left_data_type != right_data_type) {
            std::stringstream ss;
            ss << "Tried to assign expression of type '" <<  data_type_to_string(expr->data_type)
                << "' to variable of type '" << data_type_to_string(node->left->data_type) << "'";
            report_fatal_error(ss.str(), node->right->site);
        }

        assign_var(scope, name, expr);
    } else {
        reassign_var(scope, name, expr, node->right->site);
    }
}

void Interpreter::add_function_def_to_scope(Scope *scope, Ast_Function_Definition *def) {
    set_func(scope, def->name, def);
}

void Interpreter::interpret() {
    auto *global_scope = new Scope(NULL);

    Ast_Block *root = parser->parse();
    walk_from_root(global_scope, root);
}

#include <iostream>
#include <sstream>

#include "lexer.hpp"
#include "logger.hpp"
#include "parser.hpp"

std::string unspecified_parse_error(int line_number) {
    std::stringstream ss;
    ss << "Cannot parse line " << line_number << ", sorry!";
    return ss.str();
}

int get_operator_precedence(Token *token) {
    Token::Type type = token->type;
    unsigned int flags = token->flags;

    if (flags & Token::Flags::OPERATOR) {
        if (type == Token::Type::OP_MULTIPLY || type == Token::Type::OP_DIVIDE || type == Token::Type::OP_MODULO) {
            return 5;
        } else if (type == Token::Type::OP_PLUS || type == Token::Type::OP_MINUS) {
            return 4;
        }
    } else if (flags & Token::Flags::COMPARISON) {
        if (type == Token::Type::COMPARE_EQUALS || type == Token::Type::COMPARE_NOT_EQUALS) {
            return 2;
        } else {
            return 3;
        }
    } else if (flags & Token::Flags::LOGICAL) {
        if (type == Token::Type::LOGICAL_AND) {
            return 1;
        } else if (type == Token::Type::LOGICAL_OR) {
            return 0;
        } else if (type == Token::Type::LOGICAL_NOT) {
            return 6;
        }
    }

    return -1;
}

Ast_Node *Parser::parse_expression_factor() {
    Token *token = current_token;
    Token *next = peek_next_token();

    switch (token->type) {
        case Token::Type::LOGICAL_NOT:
        case Token::Type::OP_PLUS:
        case Token::Type::OP_MINUS: {
            eat(token->type);
            return new Ast_Unary_Op(token, parse_expression_factor());
        }
        case Token::Type::NUMBER:
            eat(token->type);
            return new Ast_Literal(token->value, Data_Type::NUM, token->site);
        case Token::Type::STRING:
            eat(token->type);
            return new Ast_Literal(token->value, Data_Type::STR, token->site);
        case Token::Type::KEYWORD_TRUE:
        case Token::Type::KEYWORD_FALSE: {
            eat(token->type);
            return new Ast_Literal(token->value, Data_Type::BOOL, token->site);
        }
        case Token::Type::IDENT: {
            if (next->type == Token::Type::L_PAREN) {
                return parse_function_call();
            }
            return parse_variable(false);
        }
        case Token::Type::L_PAREN: {
            eat(Token::Type::L_PAREN);
            Ast_Node *node = parse_expression();
            eat(Token::Type::R_PAREN);
            return node;
        }
        case Token::Type::L_ARRAY: {
            eat(Token::Type::L_ARRAY);
            std::vector<Ast_Node *> items;

            if (current_token->type == Token::Type::R_ARRAY) {
                eat(Token::Type::R_ARRAY);
                return new Ast_Array(items, token->site);
            }

            items.push_back(parse_expression());

            while (current_token->type == Token::Type::ARGUMENT_SEPARATOR) {
                eat(Token::Type::ARGUMENT_SEPARATOR);
                items.push_back(parse_expression());
            }

            eat(Token::Type::R_ARRAY);

            return new Ast_Array(items, token->site);
        }
        default:
            return new Ast_Empty(token->site);
    }
}

Ast_Node *Parser::parse_expression() {
    Ast_Node *left = parse_expression_factor();
    return parse_expression(left, 0);
}

Ast_Node *Parser::parse_expression(Ast_Node *left, int min_precedence) {
    while (get_operator_precedence(current_token) >= min_precedence) {
        Token *op = current_token;
        eat(Token::Flags::OPERATOR | Token::Flags::COMPARISON | Token::Flags::LOGICAL);
        Ast_Node *right = parse_expression_factor();

        if (right->node_type == Ast_Node::Type::EMPTY) report_fatal_error("Invalid operation in expression");

        while ((get_operator_precedence(current_token) > get_operator_precedence(op))
                || (current_token->flags & Token::Flags::RIGHT_TO_LEFT && get_operator_precedence(current_token) == get_operator_precedence(op))) {
            right = parse_expression(right, get_operator_precedence(current_token));
        }

        left = new Ast_Binary_Op(left, right, op);
    }

    return left;
}

Ast_Node *Parser::parse_statement() {
    Token *curr = current_token;
    Token *next = peek_next_token();
    Ast_Node *ret;

    // @CLEANUP(LOW) Mixing between checking Token types/flags when parsing statement
    // Maybe there could be some more unified data type stored on the Token to do this.
    if (curr->flags & Token::Flags::DATA_TYPE) {
        if (next->type == Token::Type::KEYWORD_FUNCTION 
                || (next->type == Token::Type::KEYWORD_ARRAY && peek_next_token(2)->type == Token::Type::KEYWORD_FUNCTION)) {
            return parse_function_definition();
        } else {
            ret = parse_assignment(true);
            eat(Token::Type::TERMINATOR);
            return ret;
        }
    }

    if (curr->type == Token::Type::L_BRACE) {
        return parse_block(false);
    } else if (curr->type == Token::Type::KEYWORD_RETURN) {
        ret = parse_return();
        eat(Token::Type::TERMINATOR);
        return ret;
    } else if (curr->type == Token::Type::KEYWORD_FUNCTION) {
        report_fatal_error("Must specify return type of bug at point of declaration", current_token->site);
        return NULL;
    } else if (curr->type == Token::Type::KEYWORD_IF) {
        return parse_if();
    } else if (curr->type == Token::Type::KEYWORD_WHILE) {
        return parse_while();
    } else if (curr->type == Token::Type::KEYWORD_LOOP_START) {
        return parse_loop();
    } else if (curr->type == Token::Type::KEYWORD_REASSIGN_VARIABLE) {
        ret = parse_assignment(false);
        eat(Token::Type::TERMINATOR);
        return ret;
    } else if (curr->type == Token::Type::IDENT && next != NULL) {
        if (next->type == Token::Type::L_PAREN) {
            ret = parse_function_call();
            eat(Token::Type::TERMINATOR);
            return ret;
        } else {
            report_fatal_error("Unexpected identifier", current_token->site);
            return NULL;
        }
    } else {
        report_fatal_error("Cannot parse this line", current_token->site);
        return NULL;
    }
}

Ast_Variable *Parser::parse_variable(bool is_first_assign) {
    Data_Type data_type = Data_Type::VOID;

    if (is_first_assign) {
        auto type = current_token->type;

        if (type == Token::Type::KEYWORD_NUM) {
            eat(Token::Type::KEYWORD_NUM);
            data_type = Data_Type::NUM;
        } else if (type == Token::Type::KEYWORD_STR) {
            eat(Token::Type::KEYWORD_STR);
            data_type = Data_Type::STR;
        } else if (type == Token::Type::KEYWORD_BOOL) {
            eat(Token::Type::KEYWORD_BOOL);
            data_type = Data_Type::BOOL;
        } else if (type == Token::Type::KEYWORD_ARRAY) {
            eat(Token::Type::KEYWORD_ARRAY);
            data_type = Data_Type::ARRAY;
        } else {
            report_fatal_error("Variables must be assigned a data type at the point of declaration", current_token->site);
        }
    }

    auto *var = new Ast_Variable(current_token);

    if (is_first_assign) var->data_type = data_type;
    if (current_token->flags & Token::Flags::KEYWORD) report_fatal_error("SHEL keyword used as variable name", current_token->site);

    eat(Token::Type::IDENT);

    return var;
}

Ast_Function_Definition *Parser::parse_function_definition() {
    Token *start_token = current_token;
    Data_Type return_type = token_to_data_type(current_token);

    eat(current_token->type);
    eat(Token::Type::KEYWORD_FUNCTION);

    std::string func_name = parse_ident_name();

    eat(Token::Type::L_PAREN);
    std::vector<Ast_Variable *> args;

    while (current_token->flags & Token::Flags::DATA_TYPE) {
        Ast_Variable *arg = parse_variable(true);

        for (Ast_Variable *other : args) {
            if (other->name == arg->name) {
                std::stringstream ss;
                ss << "Argument with the name '" << arg->name << "' already exists in definition of bug '" << func_name << "'";
                report_fatal_error(ss.str(), other->token->site);
            }
        }

        args.push_back(arg);

        if (current_token->type == Token::Type::ARGUMENT_SEPARATOR) eat(Token::Type::ARGUMENT_SEPARATOR);
    }

    if (current_token->type == Token::Type::IDENT) report_fatal_error("Must specify data type of function argument before identifier", current_token->site);

    eat(Token::Type::R_PAREN);

    Ast_Block *body = parse_block(false);

    return new Ast_Function_Definition(body, return_type, args, func_name, start_token->site);
}

Ast_Function_Call *Parser::parse_function_call() {
    Token *call_token = current_token;
    std::string func_name = parse_ident_name();

    Token *open_paren = current_token;
    eat(Token::Type::L_PAREN);
    std::vector<Ast_Node *> args;

    while (current_token->type != Token::Type::ARGUMENT_SEPARATOR && current_token->type != Token::Type::R_PAREN) {
        args.push_back(parse_expression());

        if (current_token->type == Token::Type::ARGUMENT_SEPARATOR) eat(Token::Type::ARGUMENT_SEPARATOR);
    }

    eat(Token::Type::R_PAREN);

    return new Ast_Function_Call(func_name, args, call_token->site, open_paren->site);
}

Ast_Assignment *Parser::parse_assignment(bool is_first_assign) {
    if (is_first_assign == false) {
        eat(Token::Type::KEYWORD_REASSIGN_VARIABLE);
    }

    Ast_Variable *var = parse_variable(is_first_assign);

    Token *ass_op_token = current_token;
    eat(Token::Flags::OPERATOR);
    Ast_Node *right = NULL;

    switch (ass_op_token->type) {
        case Token::Type::OP_ASSIGNMENT: 
            right = parse_expression();
            break;
        case Token::Type::OP_PLUS_EQUALS:
        case Token::Type::OP_MINUS_EQUALS:
        case Token::Type::OP_MULTIPLY_EQUALS:
        case Token::Type::OP_DIVIDE_EQUALS:
        case Token::Type::OP_MODULO_EQUALS:
            right = new Ast_Binary_Op(var, parse_expression(), ass_op_token);
            break;
        default:
            report_fatal_error("Unrecognised assignment operator", ass_op_token->site);
            return NULL;
    }

    return new Ast_Assignment(var, right, is_first_assign, ass_op_token->site);
}

Ast_Return *Parser::parse_return() {
    Token *ret_token = current_token;
    eat(Token::Type::KEYWORD_RETURN);

    return new Ast_Return(parse_expression(), ret_token->site);
}

Ast_If *Parser::parse_if() {
    Token *if_token = current_token;

    eat(Token::Type::KEYWORD_IF);
    eat(Token::Type::L_PAREN);

    Ast_Node *comparison = parse_expression();

    eat(Token::Type::R_PAREN);

    auto *root = new Ast_If(comparison, parse_block(false), if_token->site);
    auto *ret = root;

    while (current_token->type == Token::Type::KEYWORD_ELSE || current_token->type == Token::Type::KEYWORD_ELIF) {
        Code_Site *site = current_token->site;
        Ast_If *curr = NULL;

        if (current_token->type == Token::Type::KEYWORD_ELIF) {
            eat(Token::Type::KEYWORD_ELIF);
            eat(Token::Type::L_PAREN);
            Ast_Node *elif_comparison = parse_expression();
            eat(Token::Type::R_PAREN);

            curr = new Ast_If(elif_comparison, parse_block(false), site);
        } else if (current_token->type == Token::Type::KEYWORD_ELSE) {
            eat(Token::Type::KEYWORD_ELSE);
            curr = new Ast_If(NULL, parse_block(false), site);
        }

        root->failure = curr;
        root = curr;
    }

    return ret;
}

Ast_While *Parser::parse_while() {
    Token *while_token = current_token;
    eat(Token::Type::KEYWORD_WHILE);
    eat(Token::Type::L_PAREN);

    Ast_Binary_Op *comparison = (Ast_Binary_Op *)parse_expression();

    eat(Token::Type::R_PAREN);

    Ast_Block *body = parse_block(false);

    return new Ast_While(comparison, body, while_token->site);
}

Ast_Loop *Parser::parse_loop() {
    Token *loop_token = current_token;
    eat(Token::Type::KEYWORD_LOOP_START);

    Ast_Node *start = parse_expression();

    eat(Token::Type::KEYWORD_LOOP_TO);

    Ast_Node *to = parse_expression();

    eat(Token::Type::KEYWORD_LOOP_STEP);

    Ast_Node *step = parse_expression();

    Ast_Block *body = parse_block(false);

    return new Ast_Loop(start, to, step, body, loop_token->site);
}


Ast_Block *Parser::parse_block(bool is_global_scope) {
    Token *start = current_token;
    if (is_global_scope == false) eat(Token::Type::L_BRACE);
    std::vector<Ast_Node *> nodes = parse_statements();
    auto *block = new Ast_Block(nodes, start->site);

    for (Ast_Node *node : nodes) {
        // First return node in a block wins
        if (block->return_node != NULL) break;

        // @TODO(LOW) Warn if multiple returns in a block
        if (node->node_type == Ast_Node::Type::RETURN) block->return_node = (Ast_Return *)node;
    }

    if (is_global_scope == false) eat(Token::Type::R_BRACE);
    return block;
}

Ast_Block *Parser::parse() {
    if (tokens.size() == 0) return NULL;
    return parse_block(true);
}

std::string Parser::parse_ident_name() {
    std::string name = current_token->value;
    eat(Token::Type::IDENT);

    return name;
}

std::vector<Ast_Node *> Parser::parse_statements() {
    std::vector<Ast_Node *> nodes;
    Token::Type current_type = current_token->type;

    while (current_type != Token::Type::END_OF_FILE && current_type != Token::Type::R_BRACE) {
        nodes.push_back(parse_statement());
        current_type = current_token->type;
    }

    return nodes;
}

Token *Parser::peek_next_token() {
    return peek_next_token(1);
}

Token *Parser::peek_next_token(int step) {
    int pos = find(tokens.begin(), tokens.end(), current_token) - tokens.begin();
    int next_pos = pos + step;
    int tokens_len = tokens.size();

    return next_pos < tokens_len ? tokens[next_pos] : NULL;
}

void Parser::accept_or_reject_token(bool is_accepted) {
    if (is_accepted) {
        position++;
        current_token = tokens[position];
    } else {
        report_fatal_error("Unexpected token", current_token->site);
    }
}

void Parser::eat(int flags) {
    // @DUPLICATION(LOW) Parser::eat(int flags)
    if (position == tokens.size() - 1) report_fatal_error("Reached last token and attempted further eat");
    accept_or_reject_token(current_token->flags & flags);
}

void Parser::eat(Token::Type expected_type) {
    if (position == tokens.size() - 1) report_fatal_error("Reached last token and attempted further eat");
    accept_or_reject_token(current_token->type == expected_type);
}


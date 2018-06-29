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
            return parse_variable();
        }
        case Token::Type::L_PAREN: {
            eat(Token::Type::L_PAREN);
            Ast_Node *node = parse_expression();
            eat(Token::Type::R_PAREN);
            return node;
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
        ret = parse_assignment(true);
        eat(Token::Type::TERMINATOR);
        return ret;
    }

    if (curr->type == Token::Type::L_BRACE) {
        return parse_block(false);
    } else if (curr->type == Token::Type::KEYWORD_FUNCTION) {
        return parse_function_definition();
    } else if (curr->type == Token::Type::KEYWORD_RETURN) {
        ret = parse_return();
        eat(Token::Type::TERMINATOR);
        return ret;
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
    } else if (curr->type == Token::Type::IDENT && next != nullptr) {
        if (next->type == Token::Type::L_PAREN) {
            ret = parse_function_call();
            eat(Token::Type::TERMINATOR);
            return ret;
        } else {
            report_fatal_error("Unexpected identifier", current_token->site);
            return nullptr;
        }
    } else {
        report_fatal_error("Cannot parse this line", current_token->site);
        return nullptr;
    }
}

Ast_Variable *Parser::parse_variable() {
    Ast_Variable *var = new Ast_Variable(current_token);
    eat(Token::Type::IDENT);

    return var;
}

Ast_Function_Definition *Parser::parse_function_definition() {
    Token *start_token = current_token;
    eat(Token::Type::KEYWORD_FUNCTION);

    std::string func_name = parse_ident_name();

    eat(Token::Type::L_PAREN);
    std::vector<Ast_Function_Argument *> args;

    while (current_token->type == Token::Type::IDENT) {
        Token *arg_token = current_token;
        std::string arg_name = parse_ident_name();

        for (Ast_Function_Argument *other : args) {
            if (other->name == arg_name) {
                std::stringstream ss;
                ss << "Argument with the name '" << arg_name << "' already exists in definition of bug '" << func_name << "'";
                report_fatal_error(ss.str(), arg_token->site);
            }
        }

        args.push_back(new Ast_Function_Argument(arg_name, arg_token->site));

        if (current_token->type == Token::Type::ARGUMENT_SEPARATOR) eat(Token::Type::ARGUMENT_SEPARATOR);
    }

    eat(Token::Type::R_PAREN);

    Ast_Block *body = parse_block(false);

    return new Ast_Function_Definition(body, args, func_name, start_token->site);
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
    Data_Type data_type = Data_Type::UNKNOWN;

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
        } else {
            report_fatal_error("Variables must be assigned a data type at the point of declaration", current_token->site);
        }
    } else {
        eat(Token::Type::KEYWORD_REASSIGN_VARIABLE);
    }

    Ast_Variable *var = parse_variable();
    var->data_type = data_type;
    Token *ass_token = current_token;
    eat(Token::Type::OP_ASSIGNMENT);

    Ast_Node *right = parse_expression();

    return new Ast_Assignment(var, right, is_first_assign, ass_token->site);
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

    Ast_If *root = new Ast_If(comparison, parse_block(false), if_token->site);
    Ast_If *ret = root;

    while (current_token->type == Token::Type::KEYWORD_ELSE || current_token->type == Token::Type::KEYWORD_ELIF) {
        Code_Site *site = current_token->site;
        Ast_If *curr = nullptr;

        if (current_token->type == Token::Type::KEYWORD_ELIF) {
            eat(Token::Type::KEYWORD_ELIF);
            eat(Token::Type::L_PAREN);
            Ast_Node *elif_comparison = parse_expression();
            eat(Token::Type::R_PAREN);

            curr = new Ast_If(elif_comparison, parse_block(false), site);
        } else if (current_token->type == Token::Type::KEYWORD_ELSE) {
            eat(Token::Type::KEYWORD_ELSE);
            curr = new Ast_If(nullptr, parse_block(false), site);
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
    Ast_Block *block = new Ast_Block(nodes, start->site);

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
    if (tokens.size() == 0) return nullptr;
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
    return peek_next_token(current_token);
}

Token *Parser::peek_next_token(Token *current) {
    int pos = find(tokens.begin(), tokens.end(), current) - tokens.begin();
    int next_pos = pos + 1;
    int tokens_len = tokens.size();

    return next_pos < tokens_len ? tokens[next_pos] : nullptr;
}

void Parser::accept_or_reject_token(bool is_accepted) {
    if (is_accepted) {
        position++;
        current_token = tokens[position];
    } else {
        std::stringstream ss;
        ss << "Invalid syntax on line " << current_token->site->line_number;
        report_fatal_error(ss.str());
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


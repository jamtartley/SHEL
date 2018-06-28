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

Ast_Node *Parser::parse_expression_factor() {
    Token *token = current_token;
    Token *next = peek_next_token();

    switch (token->type) {
        case Token::Type::OP_PLUS:
        case Token::Type::OP_MINUS: {
            eat(token->type);
            return new Ast_Unary_Op(token, parse_expression_factor());
        }
        case Token::Type::NUMBER:
            eat(token->type);
            return new Ast_Literal(token->value, Data_Type::NUM);
        case Token::Type::STRING:
            eat(token->type);
            return new Ast_Literal(token->value, Data_Type::STR);
        case Token::Type::TRUE:
        case Token::Type::FALSE: {
            eat(token->type);
            return new Ast_Literal(token->value, Data_Type::BOOL);
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
            return new Ast_Empty();
    }
}

Ast_Node *Parser::parse_expression_term(Token *token) {
    Ast_Node *node = parse_expression_factor();

    while (current_token->type == Token::Type::OP_MULTIPLY 
            || current_token->type == Token::Type::OP_DIVIDE 
            || current_token->type == Token::Type::OP_MODULO
            || current_token->flags & Token::Flags::COMPARISON) {
        Token *token = current_token;
        eat(current_token->type);

        node = new Ast_Binary_Op(node, parse_expression_factor(), token);
    }

    return node;
}

Ast_Node *Parser::parse_expression() {
    Ast_Node *node = parse_expression_term(current_token);

    while (current_token->type == Token::Type::OP_PLUS 
        || current_token->type == Token::Type::OP_MINUS 
        || current_token->flags & Token::Flags::LOGICAL) {
        Token *token = current_token;
        eat(current_token->type);

        node = new Ast_Binary_Op(node, parse_expression_term(token), token);
    }

    return node;
}

Ast_Node *Parser::parse_statement() {
    Token *curr = current_token;
    Token *next = peek_next_token();
    Ast_Node *ret;

    if (curr->type == Token::Type::BLOCK_OPEN) {
        return parse_block(false);
    } else if (curr->type == Token::Type::KEYWORD_FUNCTION) {
        return parse_function_definition();
    } else if (curr->type == Token::Type::KEYWORD_RETURN) {
        ret = parse_return();
        eat(Token::Type::TERMINATOR);
    } else if (curr->type == Token::Type::KEYWORD_IF) {
        return parse_if();
    } else if (curr->type == Token::Type::KEYWORD_WHILE) {
        return parse_while();
    } else if (curr->type == Token::Type::KEYWORD_LOOP_START) {
        return parse_loop();
    } else if (curr->type == Token::Type::KEYWORD_ASSIGN_VARIABLE) {
        ret = parse_assignment(true);
        eat(Token::Type::TERMINATOR);
    } else if (curr->type == Token::Type::KEYWORD_REASSIGN_VARIABLE) {
        ret = parse_assignment(false);
        eat(Token::Type::TERMINATOR);
    } else if (curr->type == Token::Type::IDENT && next != nullptr) {
        if (next->type == Token::Type::L_PAREN) {
            ret = parse_function_call();
            eat(Token::Type::TERMINATOR);
        } else {
            report_fatal_error(unspecified_parse_error(current_token->line_number));
        }
    } else {
        report_fatal_error(unspecified_parse_error(current_token->line_number));
    }

    return ret;
}

Ast_Variable *Parser::parse_variable() {
    Token::Type type = current_token->type;

    Ast_Variable *var = new Ast_Variable(current_token);
    eat(Token::Type::IDENT);

    return var;
}

Ast_Function_Definition *Parser::parse_function_definition() {
    eat(Token::Type::KEYWORD_FUNCTION);
    Token::Type ret_type = current_token->type;

    std::string func_name = parse_ident_name();

    eat(Token::Type::L_PAREN);
    std::vector<Ast_Function_Argument *> args;

    while (current_token->type == Token::Type::IDENT) {
        std::string arg_name = parse_ident_name();

        for (Ast_Function_Argument *other : args) {
            if (other->name == arg_name) {
                std::stringstream ss;
                ss << "Error on line " << current_token->line_number <<  ": Argument with the name '" << arg_name << "' already exists in definition of function '" << func_name << "'";
                report_fatal_error(ss.str());
            }
        }

        args.push_back(new Ast_Function_Argument(arg_name));

        if (current_token->type == Token::Type::ARGUMENT_SEPARATOR) eat(Token::Type::ARGUMENT_SEPARATOR);
    }

    eat(Token::Type::R_PAREN);

    Ast_Block *body = parse_block(false);

    return new Ast_Function_Definition(body, args, func_name);
}

Ast_Function_Call *Parser::parse_function_call() {
    std::string func_name = parse_ident_name();

    eat(Token::Type::L_PAREN);
    std::vector<Ast_Node *> args;

    while (current_token->type != Token::Type::ARGUMENT_SEPARATOR && current_token->type != Token::Type::R_PAREN) {
        args.push_back(parse_expression());

        if (current_token->type == Token::Type::ARGUMENT_SEPARATOR) eat(Token::Type::ARGUMENT_SEPARATOR);
    }

    eat(Token::Type::R_PAREN);

    return new Ast_Function_Call(func_name, args);
}


Ast_Assignment *Parser::parse_assignment(bool is_first_assign) {
    if (is_first_assign) {
        eat(Token::Type::KEYWORD_ASSIGN_VARIABLE);
    } else {
        eat(Token::Type::KEYWORD_REASSIGN_VARIABLE);
    }

    Ast_Variable *var = parse_variable();
    eat(Token::Type::ASSIGNMENT);

    Ast_Node *right = parse_expression();

    return new Ast_Assignment(var, right, is_first_assign);
}

Ast_Return *Parser::parse_return() {
    eat(Token::Type::KEYWORD_RETURN);

    return new Ast_Return(parse_expression());
}

Ast_If *Parser::parse_if() {
    eat(Token::Type::KEYWORD_IF);
    eat(Token::Type::L_PAREN);

    Ast_Node *comparison = parse_expression();

    eat(Token::Type::R_PAREN);
    Ast_Block *success = parse_block(false);

    Ast_Block *failure = nullptr;

    if (current_token->type == Token::Type::KEYWORD_ELSE) {
        eat(Token::Type::KEYWORD_ELSE);
        failure = parse_block(false);
    }

    return new Ast_If(comparison, success, failure);
}

Ast_While *Parser::parse_while() {
    eat(Token::Type::KEYWORD_WHILE);
    eat(Token::Type::L_PAREN);

    Ast_Binary_Op *comparison = (Ast_Binary_Op *)parse_expression();

    eat(Token::Type::R_PAREN);

    Ast_Block *body = parse_block(false);

    return new Ast_While(comparison, body);
}

Ast_Loop *Parser::parse_loop() {
    eat(Token::Type::KEYWORD_LOOP_START);

    Ast_Node *start = parse_expression();

    eat(Token::Type::KEYWORD_LOOP_TO);

    Ast_Node *to = parse_expression();

    eat(Token::Type::KEYWORD_LOOP_STEP);

    Ast_Node *step = parse_expression();

    Ast_Block *body = parse_block(false);

    return new Ast_Loop(start, to, step, body);
}


Ast_Block *Parser::parse_block(bool is_global_scope) {
    if (is_global_scope == false) eat(Token::Type::BLOCK_OPEN);
    std::vector<Ast_Node *> nodes = parse_statements();
    Ast_Block *block = new Ast_Block(nodes);

    for (Ast_Node *node : nodes) {
        // First return node in a block wins
        if (block->return_node != NULL) break;

        // @TODO(LOW) Warn if multiple returns in a block
        if (node->node_type == Ast_Node::Type::RETURN) block->return_node = (Ast_Return *)node;
    }

    if (is_global_scope == false) eat(Token::Type::BLOCK_CLOSE);
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

    while (current_type != Token::Type::END_OF_FILE && current_type != Token::Type::BLOCK_CLOSE) {
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

void Parser::eat(Token::Type expected_type) {
    if (position == tokens.size() - 1) report_fatal_error("Reached last token and attempted further eat");

    if (current_token->type == expected_type) {
        position++;
        current_token = tokens[position];
    } else {
        std::stringstream ss;
        ss << "Invalid syntax on line " << current_token->line_number << ", expected '" << type_to_string(expected_type) << "', found '" << type_to_string(current_token->type) << "'.";
        report_fatal_error(ss.str());
    }
}


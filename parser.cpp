#include <iostream>
#include "lexer.hpp"
#include "parser.hpp"

void eat(Parser *parser, Token::Type expected_type) {
    if (parser->position == parser->tokens.size() - 1) std::cerr << "Reached last token and attempted further eat" << std::endl;

    if (parser->current_token->type == expected_type) {
        parser->position++;
        parser->current_token = parser->tokens[parser->position];
    } else {
        // @TODO(LOW) Exit on invalid syntax
        std::cerr << "Invalid syntax on line " << parser->current_token->line_number << ", expected '" << type_to_string(expected_type) << "', found '" << type_to_string(parser->current_token->type) << "'." << std::endl;
    }
}

Ast_Node *parse_arithmetic_factor(Parser *parser) {
    Token *token = parser->current_token;
    Token *next = peek_next_token(parser);

    switch (token->type) {
        case Token::Type::OP_PLUS:
        case Token::Type::OP_MINUS: {
            eat(parser, token->type);
            return new Ast_Unary_Op(token, parse_arithmetic_factor(parser));
        }
        case Token::Type::NUMBER:
        case Token::Type::STRING: {
            eat(parser, token->type);
            return new Ast_Literal(token->value);
        }
        case Token::Type::IDENT: {
            if (next->type == Token::Type::L_PAREN) 
                return parse_function_call(parser);
        }
        case Token::Type::L_PAREN: {
            eat(parser, Token::Type::L_PAREN);
            Ast_Node *node = parse_arithmetic_expression(parser);
            eat(parser, Token::Type::R_PAREN);
            return node;
        }
        default:
            return parse_variable(parser);
    }
}

Ast_Node *parse_arithmetic_term(Parser *parser, Token *token) {
    Ast_Node *node = parse_arithmetic_factor(parser);

    while (parser->current_token->type == Token::Type::OP_MULTIPLY || parser->current_token->type == Token::Type::OP_DIVIDE) {
        Token *token = parser->current_token;
        eat(parser, parser->current_token->type);

        node = new Ast_Binary_Op(node, parse_arithmetic_factor(parser), token);
    }

    return node;
}

Ast_Node *parse_arithmetic_expression(Parser *parser) {
    Ast_Node *node = parse_arithmetic_term(parser, parser->current_token);

    while (parser->current_token->type == Token::Type::OP_PLUS || parser->current_token->type == Token::Type::OP_MINUS) {
        Token *token = parser->current_token;
        eat(parser, parser->current_token->type);

        node = new Ast_Binary_Op(node, parse_arithmetic_term(parser, token), token);
    }

    return node;
}

Ast_Variable *parse_variable(Parser *parser) {
    Token::Type type = parser->current_token->type;

    Ast_Variable *var = new Ast_Variable(parser->current_token);
    eat(parser, Token::Type::IDENT);

    return var;
}

Ast_Function_Definition *parse_function_definition(Parser *parser) {
    eat(parser, Token::Type::KEYWORD_FUNCTION);
    Token::Type ret_type = parser->current_token->type;

    std::string name = parse_ident_name(parser);

    // @TODO(MEDIUM) Parse function args
    eat(parser, Token::Type::L_PAREN);
    eat(parser, Token::Type::R_PAREN);

    Ast_Block *body = parse_block(parser, false);

    return new Ast_Function_Definition(body, name);
}

Ast_Function_Call *parse_function_call(Parser *parser) {
    std::string name = parse_ident_name(parser);
    // @TODO(MEDIUM) Parse function args
    eat(parser, Token::Type::L_PAREN);
    eat(parser, Token::Type::R_PAREN);

    return new Ast_Function_Call(name);
}

std::string parse_ident_name(Parser *parser) {
    std::string name = parser->current_token->value;
    eat(parser, Token::Type::IDENT);

    return name;
}

Ast_Assignment *parse_assignment(Parser *parser) {
    Ast_Variable *var = parse_variable(parser);
    eat(parser, Token::Type::ASSIGNMENT);

    Ast_Node *right = parse_arithmetic_expression(parser);

    return new Ast_Assignment(var, right);
}

Ast_Block *parse_block(Parser *parser, bool is_global_scope) {
    if (is_global_scope == false) eat(parser, Token::Type::BLOCK_OPEN);
    std::vector<Ast_Node *> nodes = parse_statements(parser);
    if (is_global_scope == false) eat(parser, Token::Type::BLOCK_CLOSE);
    return new Ast_Block(nodes);
}

std::vector<Ast_Node *> parse_statements(Parser *parser) {
    std::vector<Ast_Node *> nodes;
    Token::Type current_type = parser->current_token->type;

    while (current_type != Token::Type::END_OF_FILE && current_type != Token::Type::BLOCK_CLOSE) {
        nodes.push_back(parse_statement(parser));
        current_type = parser->current_token->type;
    }

    return nodes;
}

Token *peek_next_token(Parser *parser) {
    int next_pos = parser->position + 1;
    int tokens_len = parser->tokens.size();

    return next_pos < tokens_len ? parser->tokens[next_pos] : nullptr;
}

Ast_Node *parse_statement(Parser *parser) {
    Token *curr = parser->current_token;
    Token *next = peek_next_token(parser);
    Ast_Node *ret;

    if (curr->type == Token::Type::BLOCK_OPEN) {
        return parse_block(parser, false);
    } else if (curr->type == Token::Type::KEYWORD_FUNCTION) {
        return parse_function_definition(parser);
    } else if (curr->type == Token::Type::IDENT && next != nullptr && next->type == Token::Type::L_PAREN) {
        ret = parse_function_call(parser);
        eat(parser, Token::Type::TERMINATOR);
    } else if (curr->type == Token::Type::IDENT && next != nullptr && next->type == Token::Type::ASSIGNMENT) {
        ret = parse_assignment(parser);
        eat(parser, Token::Type::TERMINATOR);
    } else {
        // @TODO(LOW) Error on not being able to parse statement into one of the above categories
    }

    return ret;
}

Ast_Block *parse(Parser *parser) {
    if (parser->tokens.size() == 0) return nullptr;
    return parse_block(parser, true);
}

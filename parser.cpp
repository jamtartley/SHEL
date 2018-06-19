#include <iostream>
#include "lexer.h"
#include "parser.h"

void eat(Parser *parser, Token::Type type) {
    if (parser->current_token->type == type) {
        parser->position++;
        parser->current_token = parser->current_token = parser->tokens[parser->position];
    } else {
        std::cerr << "Invalid syntax" << std::endl;
    }
}

Ast_Node *factor(Parser *parser, Token *token) {
    if (token->type == Token::Type::NUMBER) {
        eat(parser, Token::Type::NUMBER);
        return new Number_Node(token); 
    } else if (token->type == Token::Type::L_PAREN) {
        eat(parser, Token::Type::L_PAREN);
        Ast_Node *node = expr(parser);
        eat(parser, Token::Type::R_PAREN);
        return node;
    }

    return NULL;
}

Ast_Node *term(Parser *parser, Token *token) {
    Ast_Node *node = factor(parser, token);

    while (parser->current_token->type == Token::Type::MULTIPLY || parser->current_token->type == Token::Type::DIVIDE) {
        Token *token = parser->current_token;
        eat(parser, token->type);

        node = new Binary_Op_Node(node, factor(parser, token), token);
    }

    return node;
}

Ast_Node *expr(Parser *parser) {
    Ast_Node *node = term(parser, parser->current_token);

    while (parser->current_token->type == Token::Type::PLUS || parser->current_token->type == Token::Type::MINUS) {
        Token *token = parser->current_token;
        eat(parser, token->type);

        node = new Binary_Op_Node(node, term(parser, token), token);
    }

    return node;
}

void parse(Parser *parser) {
    if (parser->tokens.size() == 0) return;

    Ast_Node *node = expr(parser);
}

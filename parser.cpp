#include <iostream>
#include "lexer.h"
#include "parser.h"

void eat(Parser *parser, Token::Type type) {
    if (parser->position == parser->tokens.size() - 1) std::cerr << "Reached last token and attempted further eat" << std::endl;
    if (parser->current_token->type == type) {
        parser->position++;
        parser->current_token = parser->tokens[parser->position];
    } else {
        std::cerr << "Invalid syntax" << std::endl;
    }
}

Ast_Node *factor(Parser *parser) {
    Token *token = parser->current_token;

    if (token->type == Token::Type::OP_PLUS || token->type == Token::Type::OP_MINUS) {
        eat(parser, token->type);
        return new Unary_Op_Node(token, factor(parser));
    } else if (token->type == Token::Type::NUMBER) {
        eat(parser, Token::Type::NUMBER);
        return new Number_Node(token); 
    } else if (token->type == Token::Type::L_PAREN) {
        eat(parser, Token::Type::L_PAREN);
        Ast_Node *node = expr(parser);
        eat(parser, Token::Type::R_PAREN);
        return node;
    }

    return nullptr;
}

Ast_Node *term(Parser *parser, Token *token) {
    Ast_Node *node = factor(parser);

    while (parser->current_token->type == Token::Type::OP_MULTIPLY || parser->current_token->type == Token::Type::OP_DIVIDE) {
        Token *token = parser->current_token;
        eat(parser, parser->current_token->type);

        node = new Binary_Op_Node(node, factor(parser), token);
    }

    return node;
}

Ast_Node *expr(Parser *parser) {
    Ast_Node *node = term(parser, parser->current_token);

    while (parser->current_token->type == Token::Type::OP_PLUS || parser->current_token->type == Token::Type::OP_MINUS) {
        Token *token = parser->current_token;
        eat(parser, parser->current_token->type);

        node = new Binary_Op_Node(node, term(parser, token), token);
    }

    return node;
}

Variable_Node *parse_variable(Parser *parser) {
    Variable_Node *var = new Variable_Node(parser->current_token);
    eat(parser, Token::Type::IDENT);

    return var;
}

Assignment_Node *parse_assignment(Parser *parser) {
    Variable_Node *var = parse_variable(parser); 
    Token *token = parser->current_token;
    eat(parser, Token::Type::ASSIGNMENT);
    Ast_Node *right = expr(parser);

    return new Assignment_Node(var, right, token);
}

Ast_Node *parse(Parser *parser) {
    if (parser->tokens.size() == 0) return nullptr;
    Assignment_Node *ass = parse_assignment(parser);
    std::cout << ass->left->value << std::endl;
    std::cout << static_cast<Number_Node *>(ass->right)->value << std::endl;

    return ass;
}

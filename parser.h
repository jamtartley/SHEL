#ifndef PARSER_H
#define PARSER_H

#include <vector>
#include "lexer.h"

struct Parser {
    std::vector<Token *> tokens;
    Token::Type stop_type;
    Token *current_token;
    int position;

    Parser(std::vector<Token *> tokens, Token::Type stop_type) {
        this->tokens = tokens;
        this->stop_type = stop_type;
        this->current_token = tokens[0];
        this->position = 0;
    }
};

struct Ast_Node {
    enum Type {
        BINARY_OP,
        NUMBER
    };

    Type node_type;
};

struct Binary_Op_Node : Ast_Node {
    Ast_Node *left;
    Ast_Node *right;
    Token *op;

    Binary_Op_Node(Ast_Node *left, Ast_Node *right, Token *op) {
        this->left = left;
        this->right = right;
        this->op = op;
        this->node_type = Ast_Node::Type::BINARY_OP;
    }
};

struct Number_Node : Ast_Node {
    Token *token;
    float value;

    Number_Node(Token *token) {
        this->token = token;
        this->value = std::atof(token->value.c_str());
        this->node_type = Ast_Node::Type::NUMBER;
    }
};

void eat(Parser *parser, Token::Type type);
Ast_Node *factor(Parser *parser);
Ast_Node *term(Parser *parser, Token *token);
Ast_Node *expr(Parser *parser);
Ast_Node *parse(Parser *parser);

#endif

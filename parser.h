#ifndef PARSER_H
#define PARSER_H

#include <vector>
#include "lexer.h"

struct Parser {
    std::vector<Token *> tokens;
    Token::Type stop_type;

    Parser(std::vector<Token *> tokens, Token::Type stop_type) {
        this->tokens = tokens;
        this->stop_type = stop_type;
    }
};

struct Ast_Node {
};

struct Binary_Op_Node : Ast_Node {
    Ast_Node *left;
    Ast_Node *right;
    Token *op;

    Binary_Op_Node(Ast_Node *left, Ast_Node *right, Token *op) {
        this->left = left;
        this->right = right;
        this->op = op;
    }
};

struct Number_Node : Ast_Node {
    Token *token;
    float value;

    Number_Node(Token *token) {
        this->token = token;
        this->value = std::atof(token->value.c_str());
    }
};

void parse(Parser *parser);

#endif

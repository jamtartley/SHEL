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
    // @ROBUSTNESS(MEDIUM) @CLEANUP Storing type enum value in Ast_Node
    // This is a bit of a workaround for determining the type of a node
    // when only given a Ast_Node*
    enum Type {
        BINARY_OP,
        UNARY_OP,
        COMPOUND,
        ASSIGN,
        VARIABLE,
        EMPTY,
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

struct Unary_Op_Node : Ast_Node {
    Token *op;
    Ast_Node *node;

    Unary_Op_Node(Token *op, Ast_Node *node) {
        this->op = op;
        this->node = node;
        this->node_type = Ast_Node::Type::UNARY_OP;
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

struct Compound_Node : Ast_Node {
    std::vector<Ast_Node *> statements;

    Compound_Node(std::vector<Ast_Node *> statements) {
        this->statements = statements;
        this->node_type = Ast_Node::Type::COMPOUND;
    }
};

struct Variable_Node : Ast_Node {
    Token *token;
    std::string value;

    Variable_Node(Token *token) {
        this->token = token;
        this->value = token->value;
        this->node_type = Ast_Node::Type::VARIABLE;
    }
};

struct Assign_Node : Ast_Node {
    Variable_Node *left;
    Ast_Node *right;
    Token *token;

    Assign_Node(Variable_Node *left, Ast_Node *right, Token *token) {
        this->left = left;
        this->right = right;
        this->token = token;
        this->node_type = Ast_Node::Type::ASSIGN;
    }
};

struct Empty_Node : Ast_Node {
    Empty_Node() {
        this->node_type = Ast_Node::Type::EMPTY;
    }
};

void eat(Parser *parser, Token::Type type);
Ast_Node *factor(Parser *parser);
Ast_Node *term(Parser *parser, Token *token);
Ast_Node *expr(Parser *parser);
Ast_Node *parse(Parser *parser);

#endif

#ifndef PARSER_H
#define PARSER_H

#include <vector>
#include "lexer.hpp"

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
        LITERAL,
        BLOCK,
        ASSIGNMENT,
        VARIABLE,
        FUNCTION_DEFINITION,
        FUNCTION_CALL,
    };

    Type node_type;
};

struct Ast_Binary_Op : Ast_Node {
    Ast_Node *left;
    Ast_Node *right;
    Token *op;

    Ast_Binary_Op(Ast_Node *left, Ast_Node *right, Token *op) {
        this->left = left;
        this->right = right;
        this->op = op;
        this->node_type = Ast_Node::Type::BINARY_OP;
    }
};

struct Ast_Unary_Op : Ast_Node {
    Token *op;
    Ast_Node *node;

    Ast_Unary_Op(Token *op, Ast_Node *node) {
        this->op = op;
        this->node = node;
        this->node_type = Ast_Node::Type::UNARY_OP;
    }
};

struct Ast_Block : Ast_Node {
    std::vector<Ast_Node *> children;

    Ast_Block(std::vector<Ast_Node *> children) {
        this->children = children;
        this->node_type = Ast_Node::Type::BLOCK;
    }
};

struct Ast_Literal : Ast_Node {
    std::string value;

    Ast_Literal(std::string value) {
        this->value = value;
        this->node_type = Ast_Node::Type::LITERAL;
    }
}; 

struct Ast_Function_Definition : Ast_Node {
    Ast_Block *block;
    std::string name;

    Ast_Function_Definition(Ast_Block *block, std::string name) {
        this->block = block;
        this->name = name;
        this->node_type = Ast_Node::Type::FUNCTION_DEFINITION;
    }
};

struct Ast_Function_Call : Ast_Node {
    std::string name;

    Ast_Function_Call(std::string name) {
        this->name = name;
        this->node_type = Ast_Node::Type::FUNCTION_CALL;
    }
};

struct Ast_Variable : Ast_Node {
    Token *token;
    std::string name;

    Ast_Variable(Token *token) {
        this->token = token;
        this->name = token->value;
        this->node_type = Ast_Node::Type::VARIABLE;
    }
};

struct Ast_Assignment : Ast_Node {
    Ast_Variable *left;
    Ast_Node *right;

    Ast_Assignment(Ast_Variable *left, Ast_Node *right) {
        this->left = left;
        this->right = right;
        this->node_type = Ast_Node::Type::ASSIGNMENT;
    }
};

void eat(Parser *parser, Token::Type expected_type);

Ast_Node *parse_arithmetic_factor(Parser *parser);
Ast_Node *parse_arithmetic_term(Parser *parser, Token *token);
Ast_Node *parse_arithmetic_expression(Parser *parser);
Ast_Node *parse_statement(Parser *parser);

Ast_Variable *parse_variable(Parser *parser);

Ast_Function_Definition *parse_function_definition(Parser *parser);

Ast_Function_Call *parse_function_call(Parser *parser);

Ast_Assignment *parse_assignment(Parser *parser);

Ast_Block *parse_block(Parser *parser, bool is_global_scope);

std::string parse_ident_name(Parser *parser);

std::vector<Ast_Node *> parse_statements(Parser *parser);

Ast_Block *parse(Parser *parser);

Token *peek_next_token(Parser *parser);

#endif

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

enum Data_Type {
    NUM,
    STR
};

struct Ast_Node {
    // @ROBUSTNESS(MEDIUM) @CLEANUP Storing type enum value in Ast_Node
    // This is a bit of a workaround for determining the type of a node
    // when only given a Ast_Node*
    enum Type {
        BINARY_OP,
        UNARY_OP,
        BLOCK,
        ASSIGNMENT,
        VARIABLE,
        FUNCTION_DEFINITION,
        FUNCTION_CALL,
        EMPTY,
        NUMBER,
        STRING
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

struct String_Node : Ast_Node {
    Token *token;
    std::string value;

    String_Node(Token *token) {
        this->token = token;
        this->value = token->value;
        this->node_type = Ast_Node::Type::STRING;
    }
};

struct Block_Node : Ast_Node {
    std::vector<Ast_Node *> children;

    Block_Node(std::vector<Ast_Node *> children) {
        this->children = children;
        this->node_type = Ast_Node::Type::BLOCK;
    }
};

struct Function_Definition_Node : Ast_Node {
    Block_Node *block;
    std::string name;

    Function_Definition_Node(Block_Node *block, std::string name) {
        this->block = block;
        this->name = name;
        this->node_type = Ast_Node::Type::FUNCTION_DEFINITION;
    }
};

struct Function_Call_Node : Ast_Node {
    std::string name;

    Function_Call_Node(std::string name) {
        this->name = name;
        this->node_type = Ast_Node::Type::FUNCTION_CALL;
    }
};

struct Variable_Node : Ast_Node {
    Token *token;
    Data_Type type;
    std::string name;

    Variable_Node(Token *token, Token::Type token_type) {
        this->token = token;
        this->name = token->value;
        this->node_type = Ast_Node::Type::VARIABLE;

        switch (token_type) {
            default: 
            case Token::Type::KEYWORD_NUM: this->type = Data_Type::NUM; break;
            case Token::Type::KEYWORD_STR: this->type = Data_Type::STR; break;
        }
    }
};

struct Assignment_Node : Ast_Node {
    Variable_Node *left;
    Ast_Node *right;

    Assignment_Node(Variable_Node *left, Ast_Node *right) {
        this->left = left;
        this->right = right;
        this->node_type = Ast_Node::Type::ASSIGNMENT;
    }
};

struct Empty_Node : Ast_Node {
    Empty_Node() {
        this->node_type = Ast_Node::Type::EMPTY;
    }
};

void eat(Parser *parser, Token::Type expected_type);
Ast_Node *parse_arithmetic_factor(Parser *parser);
Ast_Node *parse_arithmetic_term(Parser *parser, Token *token);
Ast_Node *parse_arithmetic_expression(Parser *parser);
Variable_Node *parse_variable(Parser *parser);
Function_Definition_Node *parse_function_definition(Parser *parser);
Function_Call_Node *parse_function_call(Parser *parser);
Assignment_Node *parse_assignment(Parser *parser);
std::string parse_ident(Parser *parser);
Empty_Node *parse_empty(Parser *parser);
Block_Node *parse_block(Parser *parser, bool is_global_scope);
std::vector<Ast_Node *> parse_statements(Parser *parser);
Ast_Node *parse_statement(Parser *parser);
Block_Node *parse(Parser *parser);
Token *peek_next_token(Parser *parser);

#endif

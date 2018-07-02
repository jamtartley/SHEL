#ifndef PARSER_H
#define PARSER_H

#include <vector>
#include "lexer.hpp"
#include "typer.hpp"

struct Ast_Node {
    // @ROBUSTNESS(MEDIUM) @CLEANUP Storing type enum value in Ast_Node
    // This is a bit of a workaround for determining the type of a node
    // when only given a Ast_Node*
    enum Type {
        BINARY_OP,
        UNARY_OP,
        LITERAL,
        ARRAY,
        BLOCK,
        IF,
        WHILE,
        LOOP,
        ASSIGNMENT,
        VARIABLE,
        FUNCTION_DEFINITION,
        FUNCTION_CALL,
        RETURN,
        EMPTY
    };

    // @TODO(MEDIUM) Add flags to Ast_Node
    // Attaching some metadata to these at some point might be useful
    Type node_type;
    Data_Type data_type = Data_Type::VOID;
    Code_Site *site;
};

struct Ast_Binary_Op : Ast_Node {
    Ast_Node *left;
    Ast_Node *right;
    Token *op;

    Ast_Binary_Op(Ast_Node *left, Ast_Node *right, Token *op) {
        this->left = left;
        this->right = right;
        this->op = op;
        this->site = op->site;
        this->node_type = Ast_Node::Type::BINARY_OP;
    }
};

struct Ast_Unary_Op : Ast_Node {
    Token *op;
    Ast_Node *node;

    Ast_Unary_Op(Token *op, Ast_Node *node) {
        this->op = op;
        this->node = node;
        this->site = op->site;
        this->node_type = Ast_Node::Type::UNARY_OP;
    }
};

struct Ast_Return : Ast_Node {
    Ast_Node *value;

    Ast_Return(Ast_Node *value, Code_Site *site) {
        this->value = value;
        this->site = site;
        this->node_type = Ast_Node::Type::RETURN;
    }
};

struct Ast_Block : Ast_Node {
    std::vector<Ast_Node *> children;
    Ast_Return *return_node;

    Ast_Block(std::vector<Ast_Node *> children, Code_Site *site) {
        this->children = children;
        this->return_node = NULL;
        this->site = site;
        this->node_type = Ast_Node::Type::BLOCK;
    }
};

struct Ast_If : Ast_Node {
    Ast_Node *comparison;
    Ast_Block *success;
    Ast_If *failure;

    Ast_If(Ast_Node *comparison, Ast_Block *success, Code_Site *site) {
        this->comparison = comparison;
        this->success = success;
        this->failure = NULL; // Not known at the point of construction
        this->site = site;
        this->node_type = Ast_Node::Type::IF;
    }
};

struct Ast_While : Ast_Node {
    Ast_Node *comparison;
    Ast_Block *body;

    Ast_While(Ast_Node *comparison, Ast_Block *body, Code_Site *site) {
        this->comparison = comparison;
        this->body = body;
        this->site = site;
        this->node_type = Ast_Node::Type::WHILE;
    }
};

struct Ast_Loop : Ast_Node {
    Ast_Node *start;
    Ast_Node *to;
    Ast_Node *step;
    Ast_Block *body;

    Ast_Loop(Ast_Node *start, Ast_Node *to, Ast_Node *step, Ast_Block *body, Code_Site *site) {
        this->start = start;
        this->to = to;
        this->step = step;
        this->body = body;
        this->site = site;
        this->node_type = Ast_Node::Type::LOOP;
    }
};

struct Ast_Literal : Ast_Node {
    std::string value;

    Ast_Literal(std::string value, Data_Type type, Code_Site *site) {
        this->value = value;
        this->data_type = type;
        this->site = site;
        this->node_type = Ast_Node::Type::LITERAL;
    }
};

struct Ast_Variable : Ast_Node {
    Token *token;
    std::string name;

    // This Data_Type will be set at parse time if the variable is on the LHS of
    // an assignment, but will default to VOID if this node was parsed from
    // the RHS of an expression. The interpreter detects this and fills in/checks this
    // at interp time.
    Data_Type type;

    Ast_Variable(Token *token) {
        this->token = token;
        this->type = Data_Type::VOID;
        this->name = token->value;
        this->site = token->site;
        this->node_type = Ast_Node::Type::VARIABLE;
    }
};

struct Ast_Function_Definition : Ast_Node {
    Ast_Block *block;
    std::vector<Ast_Variable *> args;
    std::string name;

    Ast_Function_Definition(Ast_Block *block, Data_Type return_type, std::vector<Ast_Variable *> args, std::string name, Code_Site *site) {
        this->block = block;
        this->data_type = return_type;
        this->args = args;
        this->name = name;
        this->site = site;
        this->node_type = Ast_Node::Type::FUNCTION_DEFINITION;
    }
};

struct Ast_Function_Call : Ast_Node {
    std::string name;
    std::vector<Ast_Node *> args;
    Code_Site *args_start_site;

    Ast_Function_Call(std::string name, std::vector<Ast_Node *> args, Code_Site *site, Code_Site *args_start_site) {
        this->name = name;
        this->args = args;
        this->site = site;
        this->args_start_site = args_start_site;
        this->node_type = Ast_Node::Type::FUNCTION_CALL;
    }
};

struct Ast_Assignment : Ast_Node {
    Ast_Variable *left;
    Ast_Node *right;
    bool is_first_assign;

    Ast_Assignment(Ast_Variable *left, Ast_Node *right, bool is_first_assign, Code_Site *site) {
        this->left = left;
        this->right = right;
        this->is_first_assign = is_first_assign;
        this->site = site;
        this->node_type = Ast_Node::Type::ASSIGNMENT;
    }
};

struct Ast_Array : Ast_Node {
    std::vector<Ast_Node *> items;

    Ast_Array(std::vector<Ast_Node *> items, Code_Site *site) {
        this->items = items;
        this->site = site;
        this->node_type = Ast_Node::Type::ARRAY;
    }
};

struct Ast_Empty : Ast_Node {
    Ast_Empty(Code_Site *site) {
        this->site = site;
        this->data_type = Data_Type::VOID;
        this->node_type = Ast_Node::Type::EMPTY;
    }
};

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

    Ast_Node *parse_expression_factor();
    Ast_Node *parse_expression();
    Ast_Node *parse_expression(Ast_Node *left, int min_precedence);
    Ast_Node *parse_statement();
    Ast_Variable *parse_variable(bool is_first_assign);
    Ast_Function_Definition *parse_function_definition();
    Ast_Function_Call *parse_function_call();
    Ast_Assignment *parse_assignment(bool is_first_assign);
    Ast_Return *parse_return();
    Ast_If *parse_if();
    Ast_While *parse_while();
    Ast_Loop *parse_loop();
    Ast_Block *parse_block(bool is_global_scope);
    Ast_Block *parse();
    std::string parse_ident_name();
    std::vector<Ast_Node *> parse_statements();
    Token *peek_next_token();
    Token *peek_next_token(int jump);

    void accept_or_reject_token(bool is_accepted);
    void eat(int flags);
    void eat(Token::Type expected_type);
};

#endif

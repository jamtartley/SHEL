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
    Token::Type type = parser->current_token->type; // num/str

    if (parser->current_token->type == Token::Type::KEYWORD_NUM) {
        eat(parser, Token::Type::KEYWORD_NUM);
    } else if (parser->current_token->type == Token::Type::KEYWORD_STR) {
        eat(parser, Token::Type::KEYWORD_STR);
    }

    Variable_Node *var = new Variable_Node(type, parser->current_token->value);
    eat(parser, Token::Type::IDENT);

    return var;
}

Assignment_Node *parse_assignment(Parser *parser) {
    Variable_Node *var = parse_variable(parser);
    Token *token = parser->current_token;
    eat(parser, Token::Type::ASSIGNMENT);

    Ast_Node *right;
    std::cout << type_to_string(var->type) << std::endl;
    if (var->type == Token::Type::KEYWORD_NUM) {
        right = expr(parser);
    } else {
        // @ROBUSTNESS(LOW) This might not be a string? ...
        right = new String_Node(parser->current_token);
        eat(parser, Token::Type::STRING);
    }

    eat(parser, Token::Type::TERMINATOR);

    return new Assignment_Node(var, right, token);
}

Empty_Node *parse_empty(Parser *parser) {
    return new Empty_Node();
}

Compound_Node *parse_compound_statement(Parser *parser) {
    std::vector<Ast_Node *> nodes = parse_statements(parser);
    return new Compound_Node(nodes);
}

std::vector<Ast_Node *> parse_statements(Parser *parser) {
    std::vector<Ast_Node *> nodes;

    while (parser->current_token->type != Token::Type::END_OF_FILE) nodes.push_back(parse_statement(parser));
    if (parser->current_token->type == Token::Type::IDENT) std::cerr << "Unexpected identifier" << std::endl;

    return nodes;
}

Ast_Node *parse_statement(Parser *parser) {
    if (parser->current_token->type == Token::Type::KEYWORD_NUM || parser->current_token->type == Token::Type::KEYWORD_STR) {
        return parse_assignment(parser);
    } else {
        return parse_empty(parser);
    }
}

Ast_Node *parse(Parser *parser) {
    if (parser->tokens.size() == 0) return nullptr;
    Compound_Node *root = parse_compound_statement(parser);

    return root;
}

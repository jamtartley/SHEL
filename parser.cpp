#include <iostream>
#include "lexer.hpp"
#include "parser.hpp"

void eat(Parser *parser, Token::Type type) {
    if (parser->position == parser->tokens.size() - 1) std::cerr << "Reached last token and attempted further eat" << std::endl;

    if (parser->current_token->type == type) {
        parser->position++;
        parser->current_token = parser->tokens[parser->position];
    } else {
        // @TODO(LOW) Exit on invalid syntax
        std::cerr << "Invalid syntax on line: " << parser->current_token->line_number << std::endl;
    }
}

Ast_Node *parse_arithmetic_factor(Parser *parser) {
    Token *token = parser->current_token;

    if (token->type == Token::Type::OP_PLUS || token->type == Token::Type::OP_MINUS) {
        eat(parser, token->type);
        return new Unary_Op_Node(token, parse_arithmetic_factor(parser));
    } else if (token->type == Token::Type::NUMBER) {
        eat(parser, Token::Type::NUMBER);
        return new Number_Node(token);
    } else if (token->type == Token::Type::L_PAREN) {
        eat(parser, Token::Type::L_PAREN);
        Ast_Node *node = parse_arithmetic_expression(parser);
        eat(parser, Token::Type::R_PAREN);
        return node;
    } else {
        return parse_variable(parser);
    }

    return nullptr;
}

Ast_Node *parse_arithmetic_term(Parser *parser, Token *token) {
    Ast_Node *node = parse_arithmetic_factor(parser);

    while (parser->current_token->type == Token::Type::OP_MULTIPLY || parser->current_token->type == Token::Type::OP_DIVIDE) {
        Token *token = parser->current_token;
        eat(parser, parser->current_token->type);

        node = new Binary_Op_Node(node, parse_arithmetic_factor(parser), token);
    }

    return node;
}

Ast_Node *parse_arithmetic_expression(Parser *parser) {
    Ast_Node *node = parse_arithmetic_term(parser, parser->current_token);

    while (parser->current_token->type == Token::Type::OP_PLUS || parser->current_token->type == Token::Type::OP_MINUS) {
        Token *token = parser->current_token;
        eat(parser, parser->current_token->type);

        node = new Binary_Op_Node(node, parse_arithmetic_term(parser, token), token);
    }

    return node;
}

Variable_Node *parse_variable(Parser *parser) {
    Token::Type type = parser->current_token->type;

    if (type == Token::Type::KEYWORD_NUM || type == Token::Type::KEYWORD_STR) eat(parser, type);

    Variable_Node *var = new Variable_Node(parser->current_token, type);
    eat(parser, Token::Type::IDENT);

    return var;
}

Assignment_Node *parse_assignment(Parser *parser) {
    Variable_Node *var = parse_variable(parser);
    eat(parser, Token::Type::ASSIGNMENT);

    Ast_Node *right;

    if (var->type == Variable_Node::Type::NUM) {
        right = parse_arithmetic_expression(parser);
    } else {
        // @ROBUSTNESS(LOW) This might not be a string? ...
        right = new String_Node(parser->current_token);
        eat(parser, Token::Type::STRING);
    }

    eat(parser, Token::Type::TERMINATOR);

    return new Assignment_Node(var, right);
}

Empty_Node *parse_empty(Parser *parser) {
    return new Empty_Node();
}

Block_Node *parse_block_statement(Parser *parser) {
    std::vector<Ast_Node *> nodes = parse_statements(parser);
    return new Block_Node(nodes);
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

Block_Node *parse(Parser *parser) {
    if (parser->tokens.size() == 0) return nullptr;
    return parse_block_statement(parser);
}

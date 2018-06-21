#include <iostream>
#include "lexer.hpp"
#include "parser.hpp"

void eat(Parser *parser, Token::Type expected_type) {
    if (parser->position == parser->tokens.size() - 1) std::cerr << "Reached last token and attempted further eat" << std::endl;

    if (parser->current_token->type == expected_type) {
        parser->position++;
        parser->current_token = parser->tokens[parser->position];
    } else {
        // @TODO(LOW) Exit on invalid syntax
        std::cerr << "Invalid syntax on line " << parser->current_token->line_number << ", expected '" << type_to_string(expected_type) << "', found '" << type_to_string(parser->current_token->type) << "'." << std::endl;
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

Function_Definition_Node *parse_function_definition(Parser *parser) {
    eat(parser, Token::Type::KEYWORD_FUNCTION);
    std::string name = parse_ident(parser);

    // @TODO(MEDIUM) Parse function args
    eat(parser, Token::Type::L_PAREN);
    eat(parser, Token::Type::R_PAREN);

    Block_Node *body = parse_block(parser, false);

    return new Function_Definition_Node(body, name);
}

Function_Call_Node *parse_function_call(Parser *parser) {
    std::string name = parse_ident(parser);
    // @TODO(MEDIUM) Parse function args
    eat(parser, Token::Type::L_PAREN);
    eat(parser, Token::Type::R_PAREN);
    eat(parser, Token::Type::TERMINATOR);

    return new Function_Call_Node(name);
}

std::string parse_ident(Parser *parser) {
    std::string name = parser->current_token->value;
    eat(parser, Token::Type::IDENT);

    return name;
}

Assignment_Node *parse_assignment(Parser *parser) {
    Variable_Node *var = parse_variable(parser);
    eat(parser, Token::Type::ASSIGNMENT);

    Ast_Node *right;

    if (var->type == Data_Type::NUM) {
        right = parse_arithmetic_expression(parser);
    } else if (var->type == Data_Type::STR) {
        right = new String_Node(parser->current_token);
        eat(parser, Token::Type::STRING);
    } else {
        // FUNCTION
    }

    eat(parser, Token::Type::TERMINATOR);

    return new Assignment_Node(var, right);
}

Empty_Node *parse_empty(Parser *parser) {
    return new Empty_Node();
}

Block_Node *parse_block(Parser *parser, bool is_global_scope) {
    if (is_global_scope == false) eat(parser, Token::Type::BLOCK_OPEN);
    std::vector<Ast_Node *> nodes = parse_statements(parser);
    if (is_global_scope == false) eat(parser, Token::Type::BLOCK_CLOSE);
    return new Block_Node(nodes);
}

std::vector<Ast_Node *> parse_statements(Parser *parser) {
    std::vector<Ast_Node *> nodes;
    Token::Type current_type = parser->current_token->type;

    while (current_type != Token::Type::END_OF_FILE && current_type != Token::Type::BLOCK_CLOSE) {
        nodes.push_back(parse_statement(parser));
        current_type = parser->current_token->type;
    }

    return nodes;
}

Token *peek_next_token(Parser *parser) {
    int next_pos = parser->position + 1;
    int tokens_len = parser->tokens.size();

    return next_pos < tokens_len ? parser->tokens[next_pos] : nullptr;
}

Ast_Node *parse_statement(Parser *parser) {
    Token *next = peek_next_token(parser);

    if (parser->current_token->type == Token::Type::BLOCK_OPEN) {
        return parse_block(parser, false);
    } else if (parser->current_token->type == Token::Type::KEYWORD_FUNCTION) {
        return parse_function_definition(parser);
    } else if (parser->current_token->type == Token::Type::IDENT && next != nullptr && next->type == Token::Type::L_PAREN) {
        return parse_function_call(parser);
    } else if (parser->current_token->type == Token::Type::KEYWORD_NUM || parser->current_token->type == Token::Type::KEYWORD_STR) {
        return parse_assignment(parser);
    } else {
        return parse_empty(parser);
    }
}

Block_Node *parse(Parser *parser) {
    if (parser->tokens.size() == 0) return nullptr;
    return parse_block(parser, true);
}

#include <iostream>
#include <sstream>

#include "lexer.hpp"
#include "logger.hpp"
#include "parser.hpp"

std::string unspecified_parse_error(int line_number) {
    std::stringstream ss;
    ss << "Cannot parse line " << line_number << ", sorry!";
    return ss.str();
}

void eat(Parser *parser, Token::Type expected_type) {
    if (parser->position == parser->tokens.size() - 1) report_fatal_error("Reached last token and attempted further eat");

    if (parser->current_token->type == expected_type) {
        parser->position++;
        parser->current_token = parser->tokens[parser->position];
    } else {
        std::stringstream ss;
        ss << "Invalid syntax on line " << parser->current_token->line_number << ", expected '" << type_to_string(expected_type) << "', found '" << type_to_string(parser->current_token->type) << "'.";
        report_fatal_error(ss.str());
    }
}

Ast_Node *parse_arithmetic_factor(Parser *parser) {
    Token *token = parser->current_token;
    Token *next = peek_next_token(parser);

    switch (token->type) {
        case Token::Type::OP_PLUS:
        case Token::Type::OP_MINUS: {
            eat(parser, token->type);
            return new Ast_Unary_Op(token, parse_arithmetic_factor(parser));
        }
        case Token::Type::NUMBER:
        case Token::Type::STRING: {
            eat(parser, token->type);
            return new Ast_Literal(token->value);
        }
        case Token::Type::IDENT: {
            if (next->type == Token::Type::L_PAREN) {
                return parse_function_call(parser);
            }
            return parse_variable(parser);
        }
        case Token::Type::L_PAREN: {
            eat(parser, Token::Type::L_PAREN);
            Ast_Node *node = parse_arithmetic_expression(parser);
            eat(parser, Token::Type::R_PAREN);
            return node;
        }
        default:
            return parse_variable(parser);
    }
}

Ast_Node *parse_arithmetic_term(Parser *parser, Token *token) {
    Ast_Node *node = parse_arithmetic_factor(parser);

    while (parser->current_token->type == Token::Type::OP_MULTIPLY || parser->current_token->type == Token::Type::OP_DIVIDE) {
        Token *token = parser->current_token;
        eat(parser, parser->current_token->type);

        node = new Ast_Binary_Op(node, parse_arithmetic_factor(parser), token);
    }

    return node;
}

Ast_Node *parse_arithmetic_expression(Parser *parser) {
    Ast_Node *node = parse_arithmetic_term(parser, parser->current_token);

    while (parser->current_token->type == Token::Type::OP_PLUS || parser->current_token->type == Token::Type::OP_MINUS) {
        Token *token = parser->current_token;
        eat(parser, parser->current_token->type);

        node = new Ast_Binary_Op(node, parse_arithmetic_term(parser, token), token);
    }

    return node;
}

Ast_Variable *parse_variable(Parser *parser) {
    Token::Type type = parser->current_token->type;

    Ast_Variable *var = new Ast_Variable(parser->current_token);
    eat(parser, Token::Type::IDENT);

    return var;
}

Ast_Function_Definition *parse_function_definition(Parser *parser) {
    eat(parser, Token::Type::KEYWORD_FUNCTION);
    Token::Type ret_type = parser->current_token->type;

    std::string func_name = parse_ident_name(parser);

    eat(parser, Token::Type::L_PAREN);
    std::vector<Ast_Function_Argument *> args;

    while (parser->current_token->type == Token::Type::IDENT) {
        std::string arg_name = parse_ident_name(parser);

        for (Ast_Function_Argument *other : args) {
            if (other->name == arg_name) {
                std::stringstream ss;
                ss << "Error on line " << parser->current_token->line_number <<  ": Argument with the name '" << arg_name << "' already exists in definition of function '" << func_name << "'";
                report_fatal_error(ss.str());
            }
        }

        args.push_back(new Ast_Function_Argument(arg_name));

        if (parser->current_token->type == Token::Type::ARGUMENT_SEPARATOR) eat(parser, Token::Type::ARGUMENT_SEPARATOR);
    }

    eat(parser, Token::Type::R_PAREN);

    Ast_Block *body = parse_block(parser, false);

    return new Ast_Function_Definition(body, args, func_name);
}

Ast_Function_Call *parse_function_call(Parser *parser) {
    std::string func_name = parse_ident_name(parser);

    eat(parser, Token::Type::L_PAREN);
    std::vector<Ast_Node *> args;

    while (parser->current_token->type != Token::Type::ARGUMENT_SEPARATOR && parser->current_token->type != Token::Type::R_PAREN) {
        args.push_back(parse_arithmetic_expression(parser));

        if (parser->current_token->type == Token::Type::ARGUMENT_SEPARATOR) eat(parser, Token::Type::ARGUMENT_SEPARATOR);
    }

    eat(parser, Token::Type::R_PAREN);

    return new Ast_Function_Call(func_name, args);
}

std::string parse_ident_name(Parser *parser) {
    std::string name = parser->current_token->value;
    eat(parser, Token::Type::IDENT);

    return name;
}

Ast_Assignment *parse_assignment(Parser *parser, bool is_first_assign) {
    if (is_first_assign) {
        eat(parser, Token::Type::KEYWORD_ASSIGN_VARIABLE);
    } else {
        eat(parser, Token::Type::KEYWORD_REASSIGN_VARIABLE);
    }

    Ast_Variable *var = parse_variable(parser);
    eat(parser, Token::Type::ASSIGNMENT);

    Ast_Node *right = parse_arithmetic_expression(parser);

    return new Ast_Assignment(var, right, is_first_assign);
}

Ast_Return *parse_return(Parser *parser) {
    eat(parser, Token::Type::KEYWORD_RETURN);
    return new Ast_Return(parse_arithmetic_expression(parser));
}

Ast_If *parse_if(Parser *parser) {
    eat(parser, Token::Type::KEYWORD_IF);
    eat(parser, Token::Type::L_PAREN);

    Ast_Comparison *comparison = parse_comparison(parser);

    eat(parser, Token::Type::R_PAREN);
    Ast_Block *success = parse_block(parser, false);

    Ast_Block *failure = nullptr;

    if (parser->current_token->type == Token::Type::KEYWORD_ELSE) {
        eat(parser, Token::Type::KEYWORD_ELSE);
        failure = parse_block(parser, false);
    }

    return new Ast_If(comparison, success, failure);
}

Ast_While *parse_while(Parser *parser) {
    eat(parser, Token::Type::KEYWORD_WHILE);
    eat(parser, Token::Type::L_PAREN);

    Ast_Comparison *comparison = parse_comparison(parser);

    eat(parser, Token::Type::R_PAREN);

    Ast_Block *body = parse_block(parser, false);

    return new Ast_While(comparison, body);
}

Ast_Comparison *parse_comparison(Parser *parser) {
    Ast_Node *left = parse_arithmetic_expression(parser);

    // @ROBUSTNESS(LOW) Assert this is a comparator
    Token *comparator = parser->current_token;
    eat(parser, comparator->type);

    Ast_Node *right = parse_arithmetic_expression(parser);

    return new Ast_Comparison(left, right, comparator);
}

Ast_Block *parse_block(Parser *parser, bool is_global_scope) {
    if (is_global_scope == false) eat(parser, Token::Type::BLOCK_OPEN);
    std::vector<Ast_Node *> nodes = parse_statements(parser);
    Ast_Block *block = new Ast_Block(nodes);

    for (Ast_Node *node : nodes) {
        // First return node in a block wins
        if (block->return_node != NULL) break;

        // @TODO(LOW) Warn if multiple returns in a block
        if (node->node_type == Ast_Node::Type::RETURN) block->return_node = static_cast<Ast_Return *>(node);
    }

    if (is_global_scope == false) eat(parser, Token::Type::BLOCK_CLOSE);
    return block;
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
    Token *curr = parser->current_token;
    Token *next = peek_next_token(parser);
    Ast_Node *ret;

    if (curr->type == Token::Type::BLOCK_OPEN) {
        return parse_block(parser, false);
    } else if (curr->type == Token::Type::KEYWORD_FUNCTION) {
        return parse_function_definition(parser);
    } else if (curr->type == Token::Type::KEYWORD_RETURN) {
        ret = parse_return(parser);
        eat(parser, Token::Type::TERMINATOR);
    } else if (curr->type == Token::Type::KEYWORD_IF) {
        return parse_if(parser);
    } else if (curr->type == Token::Type::KEYWORD_WHILE) {
        return parse_while(parser);
    } else if (curr->type == Token::Type::KEYWORD_ASSIGN_VARIABLE) {
        ret = parse_assignment(parser, true);
        eat(parser, Token::Type::TERMINATOR);
    } else if (curr->type == Token::Type::KEYWORD_REASSIGN_VARIABLE) {
        ret = parse_assignment(parser, false);
        eat(parser, Token::Type::TERMINATOR);
    } else if (curr->type == Token::Type::IDENT && next != nullptr) {
        if (next->type == Token::Type::L_PAREN) {
            ret = parse_function_call(parser);
            eat(parser, Token::Type::TERMINATOR);
        } else {
            report_fatal_error(unspecified_parse_error(parser->current_token->line_number));
        }
    } else {
        report_fatal_error(unspecified_parse_error(parser->current_token->line_number));
    }

    return ret;
}

Ast_Block *parse(Parser *parser) {
    if (parser->tokens.size() == 0) return nullptr;
    return parse_block(parser, true);
}

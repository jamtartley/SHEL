#ifndef LEXER_H
#define LEXER_H

#include <regex>
#include <string>
#include <vector>

struct Token {
    enum Type {
        KEYWORD_IF,
        KEYWORD_ELSE,
        KEYWORD_FOR,
        KEYWORD_WHILE,
        KEYWORD_RETURN,
        KEYWORD_STRUCT,
        KEYWORD_FUNCTION,
        KEYWORD_ASSIGN_VARIABLE,
        KEYWORD_REASSIGN_VARIABLE,
        L_PAREN,
        R_PAREN,
        BLOCK_OPEN,
        BLOCK_CLOSE,
        ARGUMENT_SEPARATOR,
        OP_PLUS,
        OP_MINUS,
        OP_MULTIPLY,
        OP_DIVIDE,
        IDENT,
        NUMBER,
        STRING,
        COMPARE_EQUALS,
        COMPARE_NOT_EQUALS,
        COMPARE_LESS_THAN,
        COMPARE_GREATER_THAN,
        COMPARE_LESS_THAN_EQUALS,
        COMPARE_GREATER_THAN_EQUALS,
        ASSIGNMENT,
        TERMINATOR,
        END_OF_FILE
    };

    Token::Type type;
    std::string value;
    int line_number;
    int flags;

    Token(Token::Type type, std::string value, int line_number) {
        this->type = type;
        this->value = value;
        this->line_number = line_number;
    }
};

// @ROBUSTNESS(MEDIUM) @HACK Token::Type name maintenance nightmare
// This is a horrible way of doing this but it's not mission critical
// and it shoudn't change all that much.
inline const std::string type_to_string(Token::Type type) {
    switch (type) {
        case Token::Type::KEYWORD_IF: return "KEYWORD_IF";
        case Token::Type::KEYWORD_ELSE: return "KEYWORD_ELSE";
        case Token::Type::KEYWORD_FOR: return "KEYWORD_FOR";
        case Token::Type::KEYWORD_WHILE: return "KEYWORD_WHILE";
        case Token::Type::KEYWORD_RETURN: return "KEYWORD_RETURN";
        case Token::Type::KEYWORD_STRUCT: return "KEYWORD_STRUCT";
        case Token::Type::KEYWORD_FUNCTION: return "KEYWORD_FUNCTION";
        case Token::Type::KEYWORD_ASSIGN_VARIABLE: return "KEYWORD_ASSIGN_VARIABLE";
        case Token::Type::KEYWORD_REASSIGN_VARIABLE: return "KEYWORD_REASSIGN_VARIABLE";
        case Token::Type::L_PAREN: return "L_PAREN";
        case Token::Type::R_PAREN: return "R_PAREN";
        case Token::Type::BLOCK_OPEN: return "BLOCK_OPEN";
        case Token::Type::BLOCK_CLOSE: return "BLOCK_CLOSE";
        case Token::Type::ARGUMENT_SEPARATOR: return "ARGUMENT_SEPARATOR";
        case Token::Type::OP_PLUS: return "OP_PLUS";
        case Token::Type::OP_MINUS: return "OP_MINUS";
        case Token::Type::OP_MULTIPLY: return "OP_MULTIPLY";
        case Token::Type::OP_DIVIDE: return "OP_DIVIDE";
        case Token::Type::IDENT: return "IDENT";
        case Token::Type::NUMBER: return "NUMBER";
        case Token::Type::STRING: return "STRING";
        case Token::Type::COMPARE_EQUALS: return "COMPARE_EQUALS";
        case Token::Type::COMPARE_NOT_EQUALS: return "COMPARE_NOT_EQUALS";
        case Token::Type::COMPARE_LESS_THAN: return "COMPARE_LESS_THAN";
        case Token::Type::COMPARE_GREATER_THAN: return "COMPARE_GREATER_THAN";
        case Token::Type::COMPARE_LESS_THAN_EQUALS: return "COMPARE_LESS_THAN_EQUALS";
        case Token::Type::COMPARE_GREATER_THAN_EQUALS: return "COMPARE_GREATER_THAN_EQUALS";
        case Token::Type::ASSIGNMENT: return "ASSIGNMENT";
        case Token::Type::TERMINATOR: return "TERMINATOR";
        case Token::Type::END_OF_FILE: return "END_OF_FILE";
        default: return "INVALID TOKEN TYPE";
    }
}

std::vector<Token *> lex(const std::string source);
std::string scan_string(const std::string input, int &index, const std::regex end_match);
std::string scan_other(const std::string input, int &index, const std::regex end_match);
Token *scan_ident(const std::string input, int &index, const std::regex end_match, int line_number);
std::string peek(const std::string input, const int index);

#endif

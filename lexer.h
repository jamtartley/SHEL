#ifndef LEXER_H
#define LEXER_H

#include <regex>
#include <string>
#include <vector>

struct Token {
    enum Type {
        IDENT,
        NUMBER,
        STRING,
        KEYWORD_IF,
        KEYWORD_ELSE,
        KEYWORD_FOR,
        KEYWORD_WHILE,
        KEYWORD_NUM,
        KEYWORD_STR,
        EQUALS,
        ASSIGNMENT,
        L_PAREN,
        R_PAREN,
        L_CURLY_BRACE,
        R_CURLY_BRACE,
        TERMINATOR,
        PLUS,
        MINUS,
        MULTIPLY,
        DIVIDE,
        END_OF_FILE
    };

    Token::Type type;
    std::string value;

    Token(Token::Type type, std::string value) {
        this->type = type;
        this->value = value;
    }
};

std::vector<Token *> lex(const std::string source);
std::string scan_string(const std::string input, int &index, const std::regex end_match);
std::string scan_other(const std::string input, int &index, const std::regex end_match);
Token *scan_ident(const std::string input, int &index, const std::regex end_match);
std::string peek(const std::string input, const int index);

#endif

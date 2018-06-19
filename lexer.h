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
        EQUALS,
        L_PAREN,
        R_PAREN,
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

std::vector<Token *> lex(std::string source);
std::string scan_string(std::string input, int &index, std::regex end_match);
std::string scan_other(std::string input, int &index, std::regex end_match);

#endif

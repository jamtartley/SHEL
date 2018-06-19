#include <regex>
#include <string>
#include <vector>

struct Token {
    enum Type {
        SYMBOL,
        NUMBER,
        STRING,
        EQUALS,
        OPEN_PARENTHESES,
        CLOSE_PARENTHESES,
        STATEMENT_END,
        OPERATION
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

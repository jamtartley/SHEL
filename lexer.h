#include <regex>
#include <string>
#include <vector>

struct Token {
    enum Type {
        NUMBER,
        STRING,
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
// @CLEANUP(LOW) Don't reference index directly, bit messy
std::string read_multi_character(std::string input, int &index, std::regex end_match);

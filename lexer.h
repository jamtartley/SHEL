#include <string>
#include <vector>

struct Token {
    enum Type {
        NUMBER,
        STRING,
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
std::string read_number(std::string input);
std::string read_string(std::string input);

#include <iostream>
#include <regex>

#include "lexer.h"

std::vector<Token *> lex(std::string source) {
    static const std::regex num_regex("[.0-9]");
    static const std::regex op_regex("[+\\-*\\/]");
    std::vector<Token *> ret_tokens;

    for (std::string::size_type i = 0; i < source.size(); i++) {
        std::string current = std::string(1, source.at(i));

        if (current == " ") continue;
        if (std::regex_match(current, num_regex)) {
            ret_tokens.push_back(new Token(Token::Type::NUMBER, read_number(source.substr(i))));
            continue;
        }
        if (current == "\"") {
            ret_tokens.push_back(new Token(Token::Type::STRING, read_string(source.substr(i))));
            continue;
        }
        if (std::regex_match(current, op_regex)) {
            ret_tokens.push_back(new Token(Token::Type::OPERATION, current));
            continue;
        }

        std::cerr << "Cannot lex character: " << current << std::endl;
    }

    return ret_tokens;
}

std::string read_number(std::string input) {
    return "5";
}

std::string read_string(std::string input) {
    return "foobar";
}

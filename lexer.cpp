#include <iostream>

#include "lexer.h"

static const std::regex string_regex("[\"]");
static const std::regex num_regex("[.0-9]");
static const std::regex num_end_regex("[^.0-9]");
static const std::regex op_regex("[+\\-*\\/]");

std::vector<Token *> lex(std::string source) {
    std::vector<Token *> ret_tokens;

    for (int i = 0; i < source.size(); i++) {
        std::string current = std::string(1, source.at(i));

        if (current == " ") continue;
        if (std::regex_match(current, num_regex)) {
            ret_tokens.push_back(new Token(Token::Type::NUMBER, read_multi_character(source.substr(i), i, num_end_regex)));
            continue;
        }
        if (std::regex_match(current, string_regex)) {
            i++;
            ret_tokens.push_back(new Token(Token::Type::STRING, read_multi_character(source.substr(i), i, string_regex)));
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

std::string read_multi_character(std::string input, int &index, std::regex end_match) {
    std::string ret;

    for (int i = 0; i < input.size(); i++) {
        std::string current = std::string(1, input.at(i));

        if (std::regex_match(current, end_match)) break;

        index++;
        ret.append(current);
    }

    return ret;
}

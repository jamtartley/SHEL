#include <iostream>
#include <map>

#include "lexer.h"

static const std::regex string_regex("[\"]");
static const std::regex num_regex("[.0-9]");
static const std::regex num_end_regex("[^.0-9]");
static const std::regex op_regex("[+\\-*\\/]");

std::vector<Token *> lex(std::string source) {
    std::vector<Token *> ret_tokens;
    std::map<std::string, Token::Type> special_char_map;

    special_char_map.insert(std::make_pair("(", Token::Type::OPEN_PARENTHESES));
    special_char_map.insert(std::make_pair(")", Token::Type::CLOSE_PARENTHESES));
    special_char_map.insert(std::make_pair(";", Token::Type::STATEMENT_END));
    special_char_map.insert(std::make_pair("+", Token::Type::OPERATION));
    special_char_map.insert(std::make_pair("-", Token::Type::OPERATION));
    special_char_map.insert(std::make_pair("/", Token::Type::OPERATION));
    special_char_map.insert(std::make_pair("*", Token::Type::OPERATION));

    for (int i = 0; i < source.size(); i++) {
        std::string current = std::string(1, source.at(i));

        if (current == " ") continue;
        if (special_char_map.find(current) != special_char_map.end()) {
            ret_tokens.push_back(new Token(special_char_map[current], current));
            continue;
        }
        if (std::regex_match(current, num_regex)) {
            ret_tokens.push_back(new Token(Token::Type::NUMBER, read_multi_character(source.substr(i), i, num_end_regex)));
            continue;
        }
        if (std::regex_match(current, string_regex)) {
            i++;
            ret_tokens.push_back(new Token(Token::Type::STRING, read_multi_character(source.substr(i), i, string_regex)));
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

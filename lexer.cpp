#include <iostream>
#include <map>

#include "lexer.h"

static const std::regex string_regex("[\"]");
static const std::regex ident_start_regex("[_a-zA-Z]");
static const std::regex ident_end_regex("[^_a-zA-Z0-9]");
static const std::regex num_start_regex("[.0-9]");
static const std::regex num_end_regex("[^.0-9]");
static const std::regex op_regex("[+\\-*\\/]");

std::vector<Token *> lex(std::string source) {
    std::vector<Token *> ret_tokens;
    std::map<std::string, Token::Type> special_char_map;

    special_char_map.insert(std::make_pair("=", Token::Type::EQUALS));
    special_char_map.insert(std::make_pair("(", Token::Type::L_PAREN));
    special_char_map.insert(std::make_pair(")", Token::Type::R_PAREN));
    special_char_map.insert(std::make_pair(";", Token::Type::TERMINATOR));
    special_char_map.insert(std::make_pair("+", Token::Type::PLUS));
    special_char_map.insert(std::make_pair("-", Token::Type::MINUS));
    special_char_map.insert(std::make_pair("*", Token::Type::MULTIPLY));
    special_char_map.insert(std::make_pair("/", Token::Type::DIVIDE));

    int i = 0;

    while (i < source.size()) {
        std::string current = std::string(1, source.at(i));

        if (current == " " || current == "\n") {
            i++;
            continue;
        }
        if (special_char_map.find(current) != special_char_map.end()) {
            i++;
            ret_tokens.push_back(new Token(special_char_map[current], current));
            continue;
        }
        if (std::regex_match(current, string_regex)) {
            ret_tokens.push_back(new Token(Token::Type::STRING, scan_string(source.substr(i), i, string_regex)));
            continue;
        }
        if (std::regex_match(current, num_start_regex)) {
            ret_tokens.push_back(new Token(Token::Type::NUMBER, scan_other(source.substr(i), i, num_end_regex)));
            continue;
        }
        if (std::regex_match(current, ident_start_regex)) {
            ret_tokens.push_back(new Token(Token::Type::IDENT, scan_other(source.substr(i), i, ident_end_regex)));
            continue;
        }

        std::cerr << "Cannot lex character: " << current << std::endl;
        i++;
    }

    ret_tokens.push_back(new Token(Token::Type::END_OF_FILE, "EOF"));

    return ret_tokens;
}

std::string scan_string(std::string input, int &index, std::regex end_match) {
    std::string ret = "";
    int i = 1; // Skip first "

    while (i < input.size() && std::regex_match(std::string(1, input.at(i)), end_match) == false) {
        ret.append(std::string(1, input.at(i)));
        i++;

        if (i >= input.size()) 
            std::cerr << "String didn't terminate before end of file!" << std::endl;
    }

    index += ret.size() + 2;

    return ret;
}

std::string scan_other(std::string input, int &index, std::regex end_match) {
    std::string ret = "";
    int i = 0;

    while (i < input.size() && std::regex_match(std::string(1, input.at(i)), end_match) == false) {
        ret.append(std::string(1, input.at(i)));
        i++;
    }

    index += ret.size();

    return ret;
} 

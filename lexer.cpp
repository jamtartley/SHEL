#include <iostream>
#include <map>

#include "lexer.h"

static const std::regex string_regex("[\"]");
static const std::regex ident_start_regex("[_a-zA-Z]");
static const std::regex ident_end_regex("[^_a-zA-Z0-9]");
static const std::regex num_start_regex("[.0-9]");
static const std::regex num_end_regex("[^.0-9]");
static const std::regex op_regex("[+\\-*\\/]");

std::vector<Token *> lex(const std::string source) {
    static std::map<std::string, Token::Type> special_char_map;
    std::vector<Token *> ret_tokens;

    special_char_map.insert(std::make_pair("=", Token::Type::ASSIGNMENT));
    special_char_map.insert(std::make_pair("(", Token::Type::L_PAREN));
    special_char_map.insert(std::make_pair(")", Token::Type::R_PAREN));
    special_char_map.insert(std::make_pair("{", Token::Type::L_CURLY_BRACE));
    special_char_map.insert(std::make_pair("}", Token::Type::R_CURLY_BRACE));
    special_char_map.insert(std::make_pair(";", Token::Type::TERMINATOR));
    special_char_map.insert(std::make_pair("+", Token::Type::OP_PLUS));
    special_char_map.insert(std::make_pair("-", Token::Type::OP_MINUS));
    special_char_map.insert(std::make_pair("*", Token::Type::OP_MULTIPLY));
    special_char_map.insert(std::make_pair("/", Token::Type::OP_DIVIDE));
    special_char_map.insert(std::make_pair("<", Token::Type::COMPARE_LESS_THAN));
    special_char_map.insert(std::make_pair(">", Token::Type::COMPARE_GREATER_THAN));

    int i = 0;

    while (i < source.size()) {
        std::string current = std::string(1, source.at(i));

        if (current == " " || current == "\n") {
            i++;
            continue;
        }
        if (current == "=" && peek(source, i) == "=") {
            ret_tokens.push_back(new Token(Token::Type::COMPARE_EQUALS, "=="));
            i += 2;
            continue;
        }
        if (current == "<" && peek(source, i) == "=") {
            ret_tokens.push_back(new Token(Token::Type::COMPARE_LESS_THAN_EQUALS, "<="));
            i += 2;
            continue;
        }
        if (current == ">" && peek(source, i) == "=") {
            ret_tokens.push_back(new Token(Token::Type::COMPARE_GREATER_THAN_EQUALS, ">="));
            i += 2;
            continue;
        }
        if (current == "<" && peek(source, i) == ">") {
            ret_tokens.push_back(new Token(Token::Type::COMPARE_NOT_EQUALS, "<>"));
            i += 2;
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
            ret_tokens.push_back(scan_ident(source.substr(i), i, ident_end_regex));
            continue;
        }
        if (special_char_map.find(current) != special_char_map.end()) {
            i++;
            ret_tokens.push_back(new Token(special_char_map[current], current));
            continue;
        }

        std::cerr << "Cannot lex character: " << current << std::endl;
        i++;
    }

    ret_tokens.push_back(new Token(Token::Type::END_OF_FILE, "EOF"));

    return ret_tokens;
}

std::string scan_string(const std::string input, int &index, const std::regex end_match) {
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

std::string scan_other(const std::string input, int &index, const std::regex end_match) {
    std::string ret = "";
    int i = 0;

    while (i < input.size() && std::regex_match(std::string(1, input.at(i)), end_match) == false) {
        ret.append(std::string(1, input.at(i)));
        i++;
    }

    index += ret.size();

    return ret;
} 

Token *scan_ident(const std::string input, int &index, const std::regex end_match) {
    static std::map<std::string, Token *> keyword_map;
    // @CLEANUP(LOW) Keyword map identifier redundancies
    keyword_map.insert(std::make_pair("if", new Token(Token::Type::KEYWORD_IF, "if")));
    keyword_map.insert(std::make_pair("else", new Token(Token::Type::KEYWORD_ELSE, "else")));
    keyword_map.insert(std::make_pair("for", new Token(Token::Type::KEYWORD_FOR, "for")));
    keyword_map.insert(std::make_pair("while", new Token(Token::Type::KEYWORD_WHILE, "while")));
    keyword_map.insert(std::make_pair("num", new Token(Token::Type::KEYWORD_NUM, "num")));
    keyword_map.insert(std::make_pair("str", new Token(Token::Type::KEYWORD_STR, "str")));
    keyword_map.insert(std::make_pair("shel", new Token(Token::Type::KEYWORD_STRUCT, "shel")));
    keyword_map.insert(std::make_pair("bug", new Token(Token::Type::KEYWORD_FUNCTION, "bug")));

    std::string raw = scan_other(input, index, end_match);

    if (keyword_map.find(raw) != keyword_map.end()) return keyword_map[raw];
    return new Token(Token::Type::IDENT, raw);
}

std::string peek(const std::string input, const int index) {
    int next = index + 1;

    if (index >= input.size()) return nullptr;
    return std::string(1, input.at(next));
}

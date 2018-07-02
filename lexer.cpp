#include <iostream>
#include <map>
#include <sstream>

#include "lexer.hpp"
#include "logger.hpp"

static const std::regex string_regex("[\"]");
static const std::regex ident_start_regex("[_a-zA-Z]");
static const std::regex ident_end_regex("[^_a-zA-Z0-9]");
static const std::regex num_start_regex("[.0-9]");
static const std::regex num_end_regex("[^.0-9]");

void Lexer::lex() {
    std::vector<Token *> tokens;
    size_t file_size = file_string.size();

    while (index < file_size) {
        std::string curr = std::string(1, file_string.at(index));
        Token *next_token = NULL;
        Code_Site *site = new Code_Site(file_name, line_number, column_position);

        if (curr == " ")                                    { move_next_char(); continue; }
        else if (curr == "\n")                              { move_next_line(); continue; }
        else if (curr == "#")                               { consume_comment(); continue; }

        else if (curr == "=" && peek_next_char() != "=")    { next_token = new Token(Token::Type::OP_ASSIGNMENT, "=", site, Token::Flags::OPERATOR); }
        else if (curr == "+")                               { next_token = new Token(Token::Type::OP_PLUS, "+", site, Token::Flags::OPERATOR); }
        else if (curr == "-")                               { next_token = new Token(Token::Type::OP_MINUS, "-", site, Token::Flags::OPERATOR); }
        else if (curr == "*")                               { next_token = new Token(Token::Type::OP_MULTIPLY, "*", site, Token::Flags::OPERATOR); }
        else if (curr == "/")                               { next_token = new Token(Token::Type::OP_DIVIDE, "/", site, Token::Flags::OPERATOR); }
        else if (curr == "%")                               { next_token = new Token(Token::Type::OP_MODULO, "%", site, Token::Flags::OPERATOR); }

        else if (curr == "(")                               { next_token = new Token(Token::Type::L_PAREN, "(", site); }
        else if (curr == ")")                               { next_token = new Token(Token::Type::R_PAREN, ")", site); }
        else if (curr == "{")                               { next_token = new Token(Token::Type::L_BRACE, "{", site); }
        else if (curr == "}")                               { next_token = new Token(Token::Type::R_BRACE, "}", site); }
        else if (curr == "[")                               { next_token = new Token(Token::Type::L_ARRAY, "[", site); }
        else if (curr == "]")                               { next_token = new Token(Token::Type::R_ARRAY, "]", site); }
        else if (curr == ";")                               { next_token = new Token(Token::Type::TERMINATOR, ";", site); }
        else if (curr == ",")                               { next_token = new Token(Token::Type::ARGUMENT_SEPARATOR, ",", site); }

        else if (curr == "=" and peek_next_char() == "=")   { next_token = new Token(Token::Type::COMPARE_EQUALS, "==", site, Token::Flags::COMPARISON); }
        else if (curr == "!" and peek_next_char() == "=")   { next_token = new Token(Token::Type::COMPARE_NOT_EQUALS, "!=", site, Token::Flags::COMPARISON); }
        else if (curr == "<" and peek_next_char() == "=")   { next_token = new Token(Token::Type::COMPARE_LESS_THAN_EQUALS, "<=", site, Token::Flags::COMPARISON); }
        else if (curr == ">" and peek_next_char() == "=")   { next_token = new Token(Token::Type::COMPARE_GREATER_THAN_EQUALS, ">=", site, Token::Flags::COMPARISON); }
        else if (curr == "<")                               { next_token = new Token(Token::Type::COMPARE_LESS_THAN, "<", site, Token::Flags::COMPARISON); }
        else if (curr == ">")                               { next_token = new Token(Token::Type::COMPARE_GREATER_THAN, ">", site, Token::Flags::COMPARISON); }

        else if (std::regex_match(curr, string_regex))      { move_next_char(); next_token = new Token(Token::Type::STRING, scan_string(site, file_string.substr(index), string_regex), site, Token::Flags::LITERAL); }
        else if (std::regex_match(curr, num_start_regex))   { next_token = new Token(Token::Type::NUMBER, scan_other(site, file_string.substr(index), num_end_regex), site, Token::Flags::LITERAL); }
        else if (std::regex_match(curr, ident_start_regex)) { next_token = scan_ident(site, file_string.substr(index), ident_end_regex); }
        else {
            std::stringstream ss;
            ss << "Cannot lex character: " << curr;
            report_fatal_error(ss.str(), site);
        }

        tokens.push_back(next_token);
        move_next_chars(next_token->value.size());

        // @HACK(LOW) Consuming end string marker
        if (next_token->type == Token::Type::STRING) { move_next_char(); }
    }

    tokens.push_back(new Token(Token::Type::END_OF_FILE, "EOF", new Code_Site(file_name, line_number, column_position)));
    this->tokens = tokens;
}

void Lexer::consume_comment() {
    // @SPEED(LOW) Creating new string every character after comment found
    while (std::string(1, file_string.at(index)) != "\n") {
        move_next_char();
    }

    move_next_line();
}

void Lexer::move_next_chars(unsigned int jump) {
    index += jump;
    column_position += jump;
}

void Lexer::move_next_char() {
    move_next_chars(1);
}

void Lexer::move_next_line() {
    index++;
    line_number++;
    column_position = 1;
}

std::string Lexer::peek_next_chars(unsigned int jump) {
    unsigned int next = index + jump;

    if (index >= file_string.size()) return NULL;
    return std::string(1, file_string.at(next));
}

std::string Lexer::peek_next_char() {
    return peek_next_chars(1);
}

std::string Lexer::scan_string(Code_Site *site, const std::string input, const std::regex end_match) {
    std::string ret = "";
    int i = 0; // Skip first "
    const size_t input_size = input.size();

    while (i < input_size && std::regex_match(std::string(1, input.at(i)), end_match) == false) {
        ret.append(std::string(1, input.at(i)));

        if (i >= input_size)
            report_fatal_error("String didn't terminate before end of file!");

        i++;
    }

    return ret;
}

std::string Lexer::scan_other(Code_Site *site, const std::string input, const std::regex end_match) {
    std::string ret = "";
    int i = 0;
    const size_t input_size = input.size();

    while (i < input_size && std::regex_match(std::string(1, input.at(i)), end_match) == false) {
        ret.append(std::string(1, input.at(i)));
        i++;
    }

    return ret;
}

Token *Lexer::scan_ident(Code_Site *site, const std::string input, const std::regex end_match) {
    std::string raw = scan_other(site, input, end_match);

    if (raw == "if")          return new Token(Token::Type::KEYWORD_IF, "if", site, Token::Flags::KEYWORD);
    else if (raw == "elif")   return new Token(Token::Type::KEYWORD_ELIF, "elif", site, Token::Flags::KEYWORD);
    else if (raw == "else")   return new Token(Token::Type::KEYWORD_ELSE, "else", site, Token::Flags::KEYWORD);
    else if (raw == "while")  return new Token(Token::Type::KEYWORD_WHILE, "while", site, Token::Flags::KEYWORD);
    else if (raw == "return") return new Token(Token::Type::KEYWORD_RETURN, "return", site, Token::Flags::KEYWORD);
    else if (raw == "num")    return new Token(Token::Type::KEYWORD_NUM, "num", site, Token::Flags::KEYWORD | Token::Flags::DATA_TYPE);
    else if (raw == "str")    return new Token(Token::Type::KEYWORD_STR, "str", site, Token::Flags::KEYWORD | Token::Flags::DATA_TYPE);
    else if (raw == "bool")   return new Token(Token::Type::KEYWORD_BOOL, "bool", site, Token::Flags::KEYWORD | Token::Flags::DATA_TYPE);
    else if (raw == "arr")    return new Token(Token::Type::KEYWORD_ARRAY, "arr", site, Token::Flags::KEYWORD | Token::Flags::DATA_TYPE);
    else if (raw == "void")   return new Token(Token::Type::KEYWORD_VOID, "void", site, Token::Flags::KEYWORD | Token::Flags::DATA_TYPE);
    else if (raw == "shel")   return new Token(Token::Type::KEYWORD_STRUCT, "shel", site, Token::Flags::KEYWORD);
    else if (raw == "bug")    return new Token(Token::Type::KEYWORD_FUNCTION, "bug", site, Token::Flags::KEYWORD);
    else if (raw == "now")    return new Token(Token::Type::KEYWORD_REASSIGN_VARIABLE, "now", site, Token::Flags::KEYWORD);
    else if (raw == "from")   return new Token(Token::Type::KEYWORD_LOOP_START, "from", site, Token::Flags::KEYWORD);
    else if (raw == "to")     return new Token(Token::Type::KEYWORD_LOOP_TO, "to", site, Token::Flags::KEYWORD);
    else if (raw == "step")   return new Token(Token::Type::KEYWORD_LOOP_STEP, "step", site, Token::Flags::KEYWORD);
    else if (raw == "true")   return new Token(Token::Type::KEYWORD_TRUE, "true", site, Token::Flags::KEYWORD);
    else if (raw == "false")  return new Token(Token::Type::KEYWORD_FALSE, "false", site, Token::Flags::KEYWORD);
    else if (raw == "and")    return new Token(Token::Type::LOGICAL_AND, "and", site, Token::Flags::LOGICAL);
    else if (raw == "or")     return new Token(Token::Type::LOGICAL_OR, "or", site, Token::Flags::LOGICAL);
    else if (raw == "not")    return new Token(Token::Type::LOGICAL_NOT, "not", site, Token::Flags::LOGICAL | Token::Flags::RIGHT_TO_LEFT);
    else                      return new Token(Token::Type::IDENT, raw, site);
}

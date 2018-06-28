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
        Token *next_token = nullptr;

        if (curr == " ")                                    { move_next_char(); continue; }
        else if (curr == "\n")                              { move_next_line(); continue; }
        else if (curr == "#")                               { consume_comment(); continue; }

        else if (curr == "=" && peek_next_char() != "=")    { next_token = new Token(Token::Type::OP_ASSIGNMENT, "=", line_number, column_position, Token::Flags::OPERATOR); }
        else if (curr == "+")                               { next_token = new Token(Token::Type::OP_PLUS, "+", line_number, column_position, Token::Flags::OPERATOR); }
        else if (curr == "-")                               { next_token = new Token(Token::Type::OP_MINUS, "-", line_number, column_position, Token::Flags::OPERATOR); }
        else if (curr == "*")                               { next_token = new Token(Token::Type::OP_MULTIPLY, "*", line_number, column_position, Token::Flags::OPERATOR); }
        else if (curr == "/")                               { next_token = new Token(Token::Type::OP_DIVIDE, "/", line_number, column_position, Token::Flags::OPERATOR); }
        else if (curr == "%")                               { next_token = new Token(Token::Type::OP_MODULO, "%", line_number, column_position, Token::Flags::OPERATOR); }

        else if (curr == "(")                               { next_token = new Token(Token::Type::L_PAREN, "(", line_number, column_position); }
        else if (curr == ")")                               { next_token = new Token(Token::Type::R_PAREN, ")", line_number, column_position); }
        else if (curr == "{")                               { next_token = new Token(Token::Type::L_BRACE, "{", line_number, column_position); }
        else if (curr == "}")                               { next_token = new Token(Token::Type::R_BRACE, "}", line_number, column_position); }
        else if (curr == ";")                               { next_token = new Token(Token::Type::TERMINATOR, ";", line_number, column_position); }
        else if (curr == ",")                               { next_token = new Token(Token::Type::ARGUMENT_SEPARATOR, ",", line_number, column_position); }

        else if (curr == "=" and peek_next_char() == "=")   { next_token = new Token(Token::Type::COMPARE_EQUALS, "==", line_number, column_position, Token::Flags::COMPARISON); }
        else if (curr == "!" and peek_next_char() == "=")   { next_token = new Token(Token::Type::COMPARE_NOT_EQUALS, "!=", line_number, column_position, Token::Flags::COMPARISON); }
        else if (curr == "<" and peek_next_char() == "=")   { next_token = new Token(Token::Type::COMPARE_LESS_THAN_EQUALS, "<=", line_number, column_position, Token::Flags::COMPARISON); }
        else if (curr == ">" and peek_next_char() == "=")   { next_token = new Token(Token::Type::COMPARE_GREATER_THAN_EQUALS, ">=", line_number, column_position, Token::Flags::COMPARISON); }
        else if (curr == "<")                               { next_token = new Token(Token::Type::COMPARE_LESS_THAN, "<", line_number, column_position, Token::Flags::COMPARISON); }
        else if (curr == ">")                               { next_token = new Token(Token::Type::COMPARE_GREATER_THAN, ">", line_number, column_position, Token::Flags::COMPARISON); }

        else if (std::regex_match(curr, string_regex))      { move_next_char(); next_token = new Token(Token::Type::STRING, scan_string(file_string.substr(index), string_regex), line_number, column_position, Token::Flags::LITERAL); }
        else if (std::regex_match(curr, num_start_regex))   { next_token = new Token(Token::Type::NUMBER, scan_other(file_string.substr(index), num_end_regex), line_number, column_position, Token::Flags::LITERAL); }
        else if (std::regex_match(curr, ident_start_regex)) { next_token = scan_ident(file_string.substr(index), ident_end_regex); }
        else {
            std::stringstream ss;
            ss << "Cannot lex character: " << curr;
            report_fatal_error(ss.str());
        }

        tokens.push_back(next_token);
        move_next_chars(next_token->value.size());

        // @HACK(LOW) Consuming end string marker
        if (next_token->type == Token::Type::STRING) { move_next_char(); }
    }

    tokens.push_back(new Token(Token::Type::END_OF_FILE, "EOF", line_number, column_position));
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
    column_position = 0;
}

std::string Lexer::peek_next_chars(unsigned int jump) {
    unsigned int next = index + jump;

    if (index >= file_string.size()) return nullptr;
    return std::string(1, file_string.at(next));
}

std::string Lexer::peek_next_char() {
    return peek_next_chars(1);
}

std::string Lexer::scan_string(const std::string input, const std::regex end_match) {
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

std::string Lexer::scan_other(const std::string input, const std::regex end_match) {
    std::string ret = "";
    int i = 0;
    const size_t input_size = input.size();

    while (i < input_size && std::regex_match(std::string(1, input.at(i)), end_match) == false) {
        ret.append(std::string(1, input.at(i)));
        i++;
    }

    return ret;
}

Token *Lexer::scan_ident(const std::string input, const std::regex end_match) {
    static std::map<std::string, Token *> keyword_map;

    // @CLEANUP(LOW) Keyword map identifier redundancies
    keyword_map.insert(std::make_pair("if", new Token(Token::Type::KEYWORD_IF, "if", line_number, column_position, Token::Flags::KEYWORD)));
    keyword_map.insert(std::make_pair("else", new Token(Token::Type::KEYWORD_ELSE, "else", line_number, column_position, Token::Flags::KEYWORD)));
    keyword_map.insert(std::make_pair("while", new Token(Token::Type::KEYWORD_WHILE, "while", line_number, column_position, Token::Flags::KEYWORD)));
    keyword_map.insert(std::make_pair("return", new Token(Token::Type::KEYWORD_RETURN, "return", line_number, column_position, Token::Flags::KEYWORD)));
    keyword_map.insert(std::make_pair("shel", new Token(Token::Type::KEYWORD_STRUCT, "shel", line_number, column_position, Token::Flags::KEYWORD)));
    keyword_map.insert(std::make_pair("bug", new Token(Token::Type::KEYWORD_FUNCTION, "bug", line_number, column_position, Token::Flags::KEYWORD)));
    keyword_map.insert(std::make_pair("let", new Token(Token::Type::KEYWORD_ASSIGN_VARIABLE, "let", line_number, column_position, Token::Flags::KEYWORD)));
    keyword_map.insert(std::make_pair("relet", new Token(Token::Type::KEYWORD_REASSIGN_VARIABLE, "relet", line_number, column_position, Token::Flags::KEYWORD)));
    keyword_map.insert(std::make_pair("from", new Token(Token::Type::KEYWORD_LOOP_START, "from", line_number, column_position, Token::Flags::KEYWORD)));
    keyword_map.insert(std::make_pair("to", new Token(Token::Type::KEYWORD_LOOP_TO, "to", line_number, column_position, Token::Flags::KEYWORD)));
    keyword_map.insert(std::make_pair("step", new Token(Token::Type::KEYWORD_LOOP_STEP, "step", line_number, column_position, Token::Flags::KEYWORD)));
    keyword_map.insert(std::make_pair("true", new Token(Token::Type::KEYWORD_TRUE, "true", line_number, column_position, Token::Flags::KEYWORD)));
    keyword_map.insert(std::make_pair("false", new Token(Token::Type::KEYWORD_FALSE, "false", line_number, column_position, Token::Flags::KEYWORD)));
    keyword_map.insert(std::make_pair("and", new Token(Token::Type::COMPARE_LOGICAL_AND, "and", line_number, column_position, Token::Flags::LOGICAL)));
    keyword_map.insert(std::make_pair("or", new Token(Token::Type::COMPARE_LOGICAL_OR, "or", line_number, column_position, Token::Flags::LOGICAL)));

    std::string raw = scan_other(input, end_match);

    if (keyword_map.find(raw) != keyword_map.end()) return keyword_map[raw];
    return new Token(Token::Type::IDENT, raw, line_number, column_position);
}

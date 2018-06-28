#ifndef LEXER_H
#define LEXER_H

#include <regex>
#include <string>
#include <vector>

struct Token;

struct Lexer {
    std::vector<Token *> tokens;
    std::string file_string;

    unsigned int index = 0;
    unsigned int line_number = 1;
    unsigned int column_position = 1;

    Lexer(const std::string file_string) {
        this->file_string = file_string;
    }

    std::string scan_string(const std::string input, const std::regex end_match);
    std::string scan_other(const std::string input, const std::regex end_match);
    Token *scan_ident(const std::string input, const std::regex end_match);
    void lex();
    void consume_comment();
    void move_next_char();
    void move_next_chars(unsigned int jump);
    void move_next_line();
    std::string peek_next_chars(unsigned int jump);
    std::string peek_next_char();
};

struct Token {
    enum Type {
        KEYWORD_IF,
        KEYWORD_ELSE,
        KEYWORD_WHILE,
        KEYWORD_LOOP_START,
        KEYWORD_LOOP_TO,
        KEYWORD_LOOP_STEP,
        KEYWORD_RETURN,
        KEYWORD_STRUCT,
        KEYWORD_FUNCTION,
        KEYWORD_ASSIGN_VARIABLE,
        KEYWORD_REASSIGN_VARIABLE,
        KEYWORD_TRUE,
        KEYWORD_FALSE,
        L_PAREN,
        R_PAREN,
        L_BRACE,
        R_BRACE,
        ARGUMENT_SEPARATOR,
        OP_ASSIGNMENT,
        OP_PLUS,
        OP_MINUS,
        OP_MULTIPLY,
        OP_DIVIDE,
        OP_MODULO,
        IDENT,
        NUMBER,
        STRING,
        COMPARE_EQUALS,
        COMPARE_NOT_EQUALS,
        COMPARE_LESS_THAN,
        COMPARE_GREATER_THAN,
        COMPARE_LESS_THAN_EQUALS,
        COMPARE_GREATER_THAN_EQUALS,
        COMPARE_LOGICAL_OR,
        COMPARE_LOGICAL_AND,
        TERMINATOR,
        END_OF_FILE
    };

    enum Flags {
        NONE       = 1 << 0,
        COMPARISON = 1 << 1,
        OPERATOR   = 1 << 2,
        KEYWORD    = 1 << 3,
        LOGICAL    = 1 << 4,
        LITERAL    = 1 << 5,
    };

    Token::Type type;
    std::string value;
    unsigned int line_number;
    unsigned int column_position;
    unsigned int flags = 0;

    Token(Token::Type type, std::string value, unsigned int line_number, unsigned int column_position, Token::Flags flags = Token::Flags::NONE) {
        this->type = type;
        this->value = value;
        this->line_number = line_number;
        this->column_position = column_position;
        this->flags |= flags;
    }
};

// @ROBUSTNESS(MEDIUM) @HACK Token::Type name maintenance nightmare
// This is a horrible way of doing this but it's not mission critical
// and it shoudn't change all that much.
inline const std::string type_to_string(Token::Type type) {
    switch (type) {
        case Token::Type::KEYWORD_IF: return "KEYWORD_IF";
        case Token::Type::KEYWORD_ELSE: return "KEYWORD_ELSE";
        case Token::Type::KEYWORD_WHILE: return "KEYWORD_WHILE";
        case Token::Type::KEYWORD_LOOP_START: return "KEYWORD_LOOP_START";
        case Token::Type::KEYWORD_LOOP_TO: return "KEYWORD_LOOP_TO";
        case Token::Type::KEYWORD_LOOP_STEP: return "KEYWORD_LOOP_STEP";
        case Token::Type::KEYWORD_RETURN: return "KEYWORD_RETURN";
        case Token::Type::KEYWORD_STRUCT: return "KEYWORD_STRUCT";
        case Token::Type::KEYWORD_FUNCTION: return "KEYWORD_FUNCTION";
        case Token::Type::KEYWORD_ASSIGN_VARIABLE: return "KEYWORD_ASSIGN_VARIABLE";
        case Token::Type::KEYWORD_REASSIGN_VARIABLE: return "KEYWORD_REASSIGN_VARIABLE";
        case Token::Type::KEYWORD_TRUE: return "KEYWORD_TRUE";
        case Token::Type::KEYWORD_FALSE: return "KEYWORD_FALSE";
        case Token::Type::IDENT: return "IDENT";
        case Token::Type::NUMBER: return "NUMBER";
        case Token::Type::STRING: return "STRING";
        case Token::Type::L_PAREN: return "L_PAREN";
        case Token::Type::R_PAREN: return "R_PAREN";
        case Token::Type::L_BRACE: return "L_BRACE";
        case Token::Type::R_BRACE: return "R_BRACE";
        case Token::Type::ARGUMENT_SEPARATOR: return "ARGUMENT_SEPARATOR";
        case Token::Type::TERMINATOR: return "TERMINATOR";
        case Token::Type::END_OF_FILE: return "END_OF_FILE";
        case Token::Type::OP_ASSIGNMENT: return "OP_ASSIGNMENT";
        case Token::Type::OP_PLUS: return "OP_PLUS";
        case Token::Type::OP_MINUS: return "OP_MINUS";
        case Token::Type::OP_MULTIPLY: return "OP_MULTIPLY";
        case Token::Type::OP_DIVIDE: return "OP_DIVIDE";
        case Token::Type::OP_MODULO: return "OP_MODULO";
        case Token::Type::COMPARE_EQUALS: return "COMPARE_EQUALS";
        case Token::Type::COMPARE_NOT_EQUALS: return "COMPARE_NOT_EQUALS";
        case Token::Type::COMPARE_LESS_THAN: return "COMPARE_LESS_THAN";
        case Token::Type::COMPARE_GREATER_THAN: return "COMPARE_GREATER_THAN";
        case Token::Type::COMPARE_LESS_THAN_EQUALS: return "COMPARE_LESS_THAN_EQUALS";
        case Token::Type::COMPARE_GREATER_THAN_EQUALS: return "COMPARE_GREATER_THAN_EQUALS";
        case Token::Type::COMPARE_LOGICAL_OR: return "COMPARE_LOGICAL_OR";
        case Token::Type::COMPARE_LOGICAL_AND: return "COMPARE_LOGICAL_AND";
        default: return "INVALID TOKEN TYPE";
    }
}

#endif

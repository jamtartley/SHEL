#include <iostream>
#include <fstream>
#include <sstream>

#include "lexer.hpp"
#include "logger.hpp"
#include "parser.hpp"
#include "interp.hpp"

std::string file_to_string(std::ifstream &in) {
    std::stringstream sstr;
    sstr << in.rdbuf();
    return sstr.str();
}

void print_tokens(std::vector<Token *> tokens) {
    const int padding = 4;
    int longest_token_type = 0;

    for (Token *token : tokens) {
        // @CLEANUP(LOW) More functional style max string length from vector
        int type_len = type_to_string(token->type).size();
        if (type_len > longest_token_type) longest_token_type = type_len;
    }

    for (Token *token : tokens) {
        std::string type_string = type_to_string(token->type);
        int type_padding = longest_token_type - type_string.size() + padding;
        std::string padding_spaces(type_padding, ' ');
        std::cout << "TYPE: " << type_to_string(token->type) << padding_spaces << " VALUE: " << token->value << std::endl;
    }
}

int main(int argc, char *argv[]) {
    // @ROBUSTNESS(LOW) Improve argv control/robustness
    std::string in_file_name = argc > 1 ? argv[1] : "examples/bugs.shel";
    std::ifstream in_file(in_file_name);

    if (!in_file) {
        std::stringstream error;
        error << "Cannot open: " << in_file_name;
        report_fatal_error(error.str());
        return 1;
    }

    std::vector<Token *> tokens = lex(file_to_string(in_file));
    Parser *parser = new Parser(tokens, Token::Type::END_OF_FILE);
    Interpreter *interp = new Interpreter(parser);

    interpret(interp);

    do {
        std::cout << "Press a key to continue...";
    } while (std::cin.get() != '\n');
}

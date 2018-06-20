#include <iostream>
#include <fstream>
#include <sstream>

#include "lexer.h"
#include "parser.h"
#include "interp.h"

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

int main() {
    std::string in_file_name = "main.shel";
    std::ifstream in_file(in_file_name);

    if (!in_file) {
        std::cerr << "Cannot open: " << in_file_name << std::endl;
        return 1;
    }

    std::vector<Token *> tokens = lex(file_to_string(in_file));
    Parser *parser = new Parser(tokens, Token::Type::END_OF_FILE);
    Interpreter *interp = new Interpreter(parser);

    print_tokens(tokens);
    parse(interp->parser);
}

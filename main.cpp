#include <iostream>
#include <fstream>
#include <sstream>

#include "lexer.h"

int main() {
    std::string in_file_name = "simple.shel";
    std::ifstream in_file(in_file_name);

    if (!in_file) {
        std::cerr << "Cannot open: " <<  in_file_name << std::endl;
        return 1;
    }

    std::stringstream source_stream;
    source_stream << in_file.rdbuf();
    std::string source = source_stream.str();

    std::vector<Token *> tokens = lex(source);

    for (Token *token : tokens) {
        std::cout << "TYPE: " << token->type << " VALUE: " << token->value << std::endl;
    }
}

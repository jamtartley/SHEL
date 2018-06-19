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

int main() {
    std::string in_file_name = "simple.shel";
    std::ifstream in_file(in_file_name);

    if (!in_file) {
        std::cerr << "Cannot open: " << in_file_name << std::endl;
        return 1;
    }

    std::vector<Token *> tokens = lex(file_to_string(in_file));
    Parser *parser = new Parser(tokens, Token::Type::END_OF_FILE);

    Ast_Node *root = parse(parser);
    std::cout << "RESULT: " << walk_from_root(root) << std::endl;
}

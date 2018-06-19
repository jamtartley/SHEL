#include <iostream>

#include "lexer.h"

int main() {
    std::string source = "3 * 4";
    std::vector<Token *> tokens = lex(source);

    for (Token *token : tokens) {
        std::cout << "TYPE: " << token->type << " VALUE: " << token->value << std::endl;
    }
}

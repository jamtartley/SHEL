#include <iostream>

#include "lexer.h"

int main() {
    std::string source = "3384 (foo) * \"test\") bar 786 \"erhbef\";";
    std::vector<Token *> tokens = lex(source);

    for (Token *token : tokens) {
        std::cout << "TYPE: " << token->type << " VALUE: " << token->value << std::endl;
    }
}

#include <iostream>
#include "parser.h"

void parse(Parser *parser) {
    if (parser->tokens.size() == 0) return;

    for (int i = 0; i < parser->tokens.size(); i++) {
        Token *token = parser->tokens[i];
        std::cout << "TYPE: " << token->type << " VALUE: " << token->value << std::endl;
    }
}

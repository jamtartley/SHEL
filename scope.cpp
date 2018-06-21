#include "scope.hpp"
#include <iostream>

Var_With_Success *get_var(Scope *scope, std::string name) {
    if (is_in_scope(scope, name)) return new Var_With_Success(scope->variables[name], true);
    if (scope->parent != nullptr) return get_var(scope->parent, name);
    return nullptr;
}

void set(Scope *scope, std::string name, std::string value) {
    scope->variables[name] = value;
}

bool is_in_scope(Scope *scope, std::string name) {
    return (scope->variables.find(name) != scope->variables.end());
}

void print_contents(Scope *scope) {
    for (auto const &k : scope->variables) {
        std::cout << k.first << ": " << k.second << std::endl;
    }
}

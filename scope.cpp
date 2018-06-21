#include <iostream>
#include "scope.hpp"

Var_With_Success *get_var(Scope *scope, std::string name) {
    if (is_var_in_scope(scope, name)) return new Var_With_Success(scope->variables[name], true);
    if (scope->parent != nullptr) return get_var(scope->parent, name);
    return nullptr;
}

Func_With_Success *get_func(Scope *scope, std::string name) {
    if (is_func_in_scope(scope, name)) return new Func_With_Success(scope->functions[name], true);
    if (scope->parent != nullptr) return get_func(scope->parent, name);
    return nullptr;
}

void set_var(Scope *scope, std::string name, std::string value) {
    scope->variables[name] = value;
}

void set_func(Scope *scope, std::string name, Function_Definition_Node *func) {
    scope->functions[name] = func;
}

bool is_var_in_scope(Scope *scope, std::string name) {
    return (scope->variables.find(name) != scope->variables.end());
}

bool is_func_in_scope(Scope *scope, std::string name) {
    return (scope->functions.find(name) != scope->functions.end());
}

void print_contents(Scope *scope) {
    std::cout << "SCOPE (DEPTH - " << scope->depth << ")" << std::endl;

    for (auto const &k : scope->variables) {
        std::cout << k.first << ": " << k.second << std::endl;
    }

    for (auto const &k : scope->functions) {
        std::cout << k.first << ": " << k.second << std::endl;
    }
}

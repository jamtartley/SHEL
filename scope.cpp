#include <iostream>
#include <sstream>

#include "logger.hpp"
#include "scope.hpp"

// @CLEANUP(LOW) Lots of repetitions between vars/functions in scope.cpp

Var_With_Success *get_var(Scope *scope, std::string name) {
    if (is_var_in_scope(scope, name)) return new Var_With_Success(scope->variables[name], true);
    if (scope->parent != NULL) return get_var(scope->parent, name);
    return new Var_With_Success(NULL, false);
}

Func_With_Success *get_func(Scope *scope, std::string name) {
    if (is_func_in_scope(scope, name)) return new Func_With_Success(scope->functions[name], true);
    if (scope->parent != NULL) return get_func(scope->parent, name);
    return new Func_With_Success(NULL, false);
}

void assign_var(Scope *scope, std::string name, Data_Atom *value) {
    scope->variables[name] = value;
}

void reassign_var(Scope *scope, std::string name, Data_Atom *value, Code_Site *site) {
    Scope *current = scope;

    // Move up scopes looking for a variable of the given name to reassign
    while (current != NULL) {
        if (is_var_in_scope(current, name)) {
            if (current->variables[name]->data_type != value->data_type) {
                std::stringstream ss;
                ss << "Tried to reassign variable of type '" <<  data_type_to_string(current->variables[name]->data_type)
                    << "' to expression of type '" << data_type_to_string(value->data_type) << "'";
                report_fatal_error(ss.str(), site);
            }

            current->variables[name] = value;
            return;
        }

        current = current->parent;
    }

    std::stringstream ss;
    ss << "Attempted to reassign variable with the name '" << name << "', but none by that name exists.";
    report_fatal_error(ss.str());
}

void set_func(Scope *scope, std::string name, Ast_Function_Definition *func) {
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

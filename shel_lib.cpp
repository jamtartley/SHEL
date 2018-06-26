#include <iostream>

#include "shel_lib.hpp"

void print_native(std::string main_string, std::vector<std::string> others) {
    std::string build = main_string;

    for (auto other : others) {
        size_t start_pos = build.find("%");

        if (start_pos == std::string::npos) std::cerr << "Wrong number of args passed to print" << std::endl;

        build.replace(start_pos, 1, other);
    }

    std::cout << build << std::endl;
}

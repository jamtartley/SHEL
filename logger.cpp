#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>

#include "lexer.hpp"

void report_fatal_error(std::string error);

std::string get_line_by_number(Code_Site *site) {
    std::ifstream in_file(site->file_name);

    if (!in_file) {
        std::stringstream error;
        error << "Cannot open: " << site->file_name;
        report_fatal_error(error.str());
        return "";
    }

    std::string ret;

    for (int i = 0; i < site->line_number; i++) {
        std::getline(in_file, ret);
    }

    return ret;
}

std::string get_column_marker(std::string line, int column) {
    std::string before(column - 1, '_');
    std::string marker = "^";

    return before + marker;
}

void report_fatal_error(std::string error) {
    std::cerr << "[FATAL ERROR] " << error << std::endl;
    std::cerr << "Exiting..." << std::endl;

    exit(0);
}

void report_fatal_error(std::string error, Code_Site *site) {
    std::string line_of_error;
    std::string line_string = get_line_by_number(site);

    std::cerr << "[FATAL ERROR] at line " << site->line_number << ", column " << site->column_position << ": " << error << std::endl;
    std::cerr << std::endl;
    std::cerr << line_string << std::endl;
    std::cerr << get_column_marker(line_string, site->column_position) << std::endl;
    std::cerr << std::endl;
    std::cerr << "Exiting..." << std::endl;

    exit(0);
}

void report_warning(std::string warning) {
    std::cerr << "[WARNING] " << warning << std::endl;
}

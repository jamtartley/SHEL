#include <cstdlib>
#include <iostream>

#include "logger.hpp"

void report_fatal_error(std::string error) {
    std::cerr << "[FATAL ERROR] " << error << std::endl;
    std::cerr << "Exiting..." << std::endl;

    exit(0);
}

void report_warning(std::string warning) {
    std::cerr << "[WARNING] " << warning << std::endl;
}

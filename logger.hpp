#include <string>

#include "lexer.hpp"

void report_fatal_error(std::string error);
void report_fatal_error(std::string error, Code_Site *site);
void report_warning(std::string warning);

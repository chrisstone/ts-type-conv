#pragma once

#include <istream>
#include "ast.h"

std::unique_ptr<ast::file> parse_file(std::istream& input);

#pragma once

#include <iosfwd>
#include <map>
#include <string>
#include "../ast.h"

#include "../config.h"

void generate_cpp(std::ostream& out, ast::file* file, const codegen_config& config);

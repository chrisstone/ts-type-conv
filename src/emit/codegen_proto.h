#pragma once

#include <iosfwd>
#include "../ast.h"
#include "../config.h"

void generate_proto(std::ostream& out, ast::file* file, const codegen_config& config);

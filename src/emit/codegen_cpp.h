#pragma once

#include <iosfwd>
#include <map>
#include <string>
#include "../ast.h"

struct datatype_config {
    std::string out;
    std::string header;
};

enum class enum_generation_mode {
    standard,
    with_array
};

struct cpp_config {
    enum_generation_mode enum_mode = enum_generation_mode::standard;
};

struct codegen_config {
    std::map<std::string, datatype_config> datatypes;
    cpp_config cpp;
};

void generate_cpp(std::ostream& out, ast::file* file, const codegen_config& config);

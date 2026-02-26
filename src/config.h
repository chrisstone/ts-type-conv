#pragma once

#include <iosfwd>
#include <map>
#include <string>

/**
 * @brief Configuration relating to specific data types and overriding their output generation.
 */
struct datatype_config {
    std::string out;
    std::string header;
};

/**
 * @brief Modes for enum code generation.
 */
enum class enum_generation_mode {
    standard,  /*!< Standard C++ enum class representation */
    with_array /*!< Generates an array of all possible enum values alongside the enum */
};

/**
 * @brief Settings specific to C++ code generation output.
 */
struct cpp_config {
    enum_generation_mode enum_mode = enum_generation_mode::standard;
};

/**
 * @brief Centralized configuration options for the ts-type-conv code generation processes.
 */
struct codegen_config {
    std::map<std::string, datatype_config> datatypes;
    cpp_config cpp;
};

/**
 * @brief Parses a TOML configuration file into the codegen_config struct and determines the requested output format.
 *
 * @param config_file The path to the configuration file, typically a .toml file. If empty, uses default settings.
 * @param conf The codegen_config structure to populate with settings from the configuration file.
 * @param string_format An output parameter that will be populated with the requested format string (e.g., "cpp", "proto").
 * @return True if parsing succeeded or there was no config_file, false if a parsing error occurred.
 */
bool parse_config(const std::string& config_file, codegen_config& conf, std::string& string_format);

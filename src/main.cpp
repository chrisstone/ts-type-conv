
#include <fstream>
#include <iostream>
#include <string>

#include "parser.h"
#include "emit/codegen_cpp.h"
#include "emit/codegen_proto.h"
#include "toml.hpp"

static void print_help(const char* exe_name)
{
    std::cout << "Usage: " << exe_name << " <input_file | -> <output_file | -> [config.toml]\n"
              << "   - indicates stdin for input or stdout for output.\n";
}

int main(int argc, char** argv)
{
    if (argc < 3)
    {
        std::cerr << "ERROR: Invalid parameters.\n";
        print_help(argv[0]);
        return 1;
    }

    std::string in_file = argv[1];
    std::string out_file = argv[2];
    std::string config_file;

    if (argc > 3)
    {
        config_file = argv[3];
    }

    std::string string_format = "cpp";
    codegen_config conf;

	// Process Config File
	if (!config_file.empty())
    {
        try
        {
            toml::table tbl = toml::parse_file(config_file);

            if (auto format_val = tbl.get("format"))
            {
                if (auto format_str = format_val->value<std::string>())
                {
                    if (*format_str != "cpp" && *format_str != "proto")
                    {
                        std::cerr << "ERROR: format '" << *format_str << "' is not supported. Only 'cpp' and 'proto' formts are supported.\n";
                        return 1;
                    }
                    string_format = *format_str;
                }
            }

            if (auto cpp_tbl = tbl["cpp"].as_table())
            {
                if (auto enum_val = cpp_tbl->get("enum"))
                {
                    if (auto enum_str = enum_val->value<std::string>())
                    {
                        if (*enum_str == "withArray") conf.cpp.enum_mode = enum_generation_mode::with_array;
                    }
                }
            }

            if (auto datatypes = tbl["datatype"].as_table())
            {
                for (auto& kv : *datatypes)
                {
                    if (auto dt_table = kv.second.as_table())
                    {
                        datatype_config entry;
                        if (auto out_val = dt_table->get("out"))
                        {
                            if (auto out_str = out_val->value<std::string>()) entry.out = *out_str;
                        }
                        if (auto h_val = dt_table->get("header"))
                        {
                            if (auto header_str = h_val->value<std::string>()) entry.header = *header_str;
                        }
                        conf.datatypes[std::string(kv.first.str())] = entry;
                    }
                }
            }
        }
        catch (const toml::parse_error& err)
        {
            std::cerr << "ERROR parsing config file: " << err << "\n";
            return 1;
        }
    }

	// Open Input Stream
    std::istream* in_stream = &std::cin;
    std::ifstream input_file_stream;
    if (in_file != "-")
    {
        input_file_stream.open(in_file);
        if (input_file_stream.fail())
        {
            std::cerr << "ERROR: Failed to open file '" << in_file << "'\n";
            return 1;
        }
        in_stream = &input_file_stream;
    }

	// Open Output Stream
    std::ostream* out_stream = &std::cout;
    std::ofstream output_file_stream;
    if (out_file != "-")
    {
        output_file_stream.open(out_file);
        if (output_file_stream.fail())
        {
            std::cerr << "ERROR: Failed to open output file '" << out_file << "'\n";
            return 1;
        }
        out_stream = &output_file_stream;
    }

	// Parse Input File
    auto file = parse_file(*in_stream);
    if (!file)
    {
        std::cerr << "Error encountered while parsing file; aborting\n";
        return 1;
    }
    if (string_format == "proto") {
        generate_proto(*out_stream, file.get(), conf);
    } else {
        *out_stream << "/* Generated C++ Header from " << (in_file == "-" ? "stdin" : in_file) << " */\n";
        generate_cpp(*out_stream, file.get(), conf);
    }

    return 0;
}

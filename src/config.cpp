#include "config.h"

#include <iostream>
#include "toml.hpp"

bool parse_config(const std::string& config_file, codegen_config& conf, std::string& string_format)
{
    if (config_file.empty())
    {
        return true;
    }

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
                    return false;
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
        return false;
    }

    return true;
}

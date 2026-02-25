#pragma once

#include <fstream>

#include "ast.h"

enum class token
{
    // State values
    invalid,
    eof,

    // Characters
    open_curly, // {
    close_curly, // }
    open_bracket, // [
    close_bracket, // ]
    open_paren, // (
    close_paren, // )
    semicolon, // ;
    colon, // :
    question, // ?
    pipe, // |
    equals, // =
    less_than, // <
    greater_than, // >
    ampersand, // &
    comma, // ,
    dot, // .
    backtick, // `

    // Keywords
    keyword_export,
    keyword_module,
    keyword_interface,
    keyword_extends,
    keyword_type,
    keyword_keyof,
    keyword_in,
    keyword_unknown,
    keyword_never,
    keyword_import,
    keyword_from,
    keyword_enum,

    // Types
    type_any,
    type_boolean,
    type_string,
    type_number,

    // Arbitrary string values
    string,
    identifier,
    number_literal,
};

struct lexer
{
    lexer(std::istream& input, ast::file* file) : input(input), file(file) { advance(); }

    explicit operator bool() const noexcept
    {
        return (current_token != token::eof) && (current_token != token::invalid);
    }

    void advance();

    std::istream& input;
    ast::file* file;
    token current_token = token::invalid;
    std::string string_value;
};

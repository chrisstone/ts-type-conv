
#include <cassert>

#include "lexer.h"
#include "parser.h"

using namespace std::literals;

static ast::node* parse_export(lexer& lex);
static ast::object* parse_object(lexer& lex);
static ast::node* parse_type_reference(lexer& lex);
static ast::node* parse_single_type(lexer& lex);
static ast::type_alias* parse_type_alias(lexer& lex);
static ast::import_stmt* parse_import(lexer& lex);
static ast::enumeration* parse_enum(lexer& lex);

static ast::module* parse_module(lexer& lex)
{
    assert(lex.current_token == token::keyword_module);
    lex.advance();

    if (lex.current_token != token::identifier)
    {
        std::printf("ERROR: Unexpected token '%s' for name of module; expected an identifier\n", lex.string_value.c_str());
        return nullptr;
    }

    auto result = std::make_unique<ast::module>();
    result->name.swap(lex.string_value);

    lex.advance();
    if (lex.current_token != token::open_curly)
    {
        std::printf("ERROR: Unexpected token '%s' after declaration of module '%s'; expected an '{'\n", lex.string_value.c_str(), result->name.c_str());
        return nullptr;
    }

    lex.advance();
    while (lex.current_token != token::close_curly)
    {
        switch (lex.current_token)
        {
        case token::keyword_export:
        {
            auto ptr = parse_export(lex);
            if (!ptr)
            {
                std::printf("NOTE: While processing module '%s'\n", result->name.c_str());
                return nullptr;
            }
            ptr->parent = result.get();
            result->children.push_back(std::move(ptr));
        }   break;

        default:
            std::printf("ERROR: Unexpected token '%s' while parsing module '%s' body\n", lex.string_value.c_str(), result->name.c_str());
            return nullptr;
        }
    }

    lex.advance(); // Consume the '}'

    auto resultPtr = result.get();
    lex.file->nodes.push_back(std::move(result));
    return resultPtr;
}

static ast::node* parse_single_type(lexer& lex)
{
    ast::node* result = nullptr;
    switch (lex.current_token)
    {
    case token::type_string:
        result = new ast::fundamental_type_reference(ast::fundamental_type::string);
        lex.file->nodes.emplace_back(result);
        lex.advance();
        break;
    case token::type_boolean:
        result = new ast::fundamental_type_reference(ast::fundamental_type::boolean);
        lex.file->nodes.emplace_back(result);
        lex.advance();
        break;
    case token::type_number:
        result = new ast::fundamental_type_reference(ast::fundamental_type::number);
        lex.file->nodes.emplace_back(result);
        lex.advance();
        break;
    case token::type_any:
        result = new ast::fundamental_type_reference(ast::fundamental_type::any);
        lex.file->nodes.emplace_back(result);
        lex.advance();
        break;
    case token::keyword_unknown:
        result = new ast::fundamental_type_reference(ast::fundamental_type::unknown);
        lex.file->nodes.emplace_back(result);
        lex.advance();
        break;
    case token::keyword_never:
        result = new ast::fundamental_type_reference(ast::fundamental_type::never);
        lex.file->nodes.emplace_back(result);
        lex.advance();
        break;
    case token::string:
    case token::number_literal:
    case token::backtick:
    {
        auto lit = std::make_unique<ast::literal_type>();
        lit->value = lex.string_value;
        lit->is_string = (lex.current_token == token::string) || (lex.current_token == token::backtick);
        lit->is_number = (lex.current_token == token::number_literal);
        result = lit.get();
        lex.file->nodes.push_back(std::move(lit));
        lex.advance();
        break;
    }
    case token::open_curly:
    {
        result = parse_object(lex);
        break;
    }
    case token::open_bracket:
    {
        lex.advance();
        auto tup = std::make_unique<ast::tuple_type>();
        while (lex.current_token != token::close_bracket && lex.current_token != token::eof)
        {
            if (lex.current_token == token::dot)
            {
                lex.advance();
                if (lex.current_token == token::dot) lex.advance();
                if (lex.current_token == token::dot) lex.advance();
            }

            if (lex.current_token == token::identifier)
            {
                std::string peek = lex.string_value;
                lex.advance();
                if (lex.current_token == token::colon)
                {
                    lex.advance();
                }
                else
                {

                }
            }

            ast::node* elem_type = parse_type_reference(lex);
            if (elem_type)
            {
                elem_type->parent = tup.get();
                tup->elements.push_back(elem_type);
            }

            if (lex.current_token == token::comma)
            {
                lex.advance();
            }
        }
        if (lex.current_token == token::close_bracket) lex.advance();
        result = tup.get();
        lex.file->nodes.push_back(std::move(tup));
        break;
    }
    case token::identifier:
    case token::keyword_keyof:
    {
        auto ref = std::make_unique<ast::generic_type_reference>();
        ref->name.swap(lex.string_value);
        result = ref.get();
        auto refPtr = ref.get();
        lex.file->nodes.push_back(std::move(ref));
        lex.advance();

        if (lex.current_token == token::less_than)
        {
            lex.advance();
            while (lex.current_token != token::greater_than && lex.current_token != token::eof)
            {
                ast::node* arg = parse_type_reference(lex);
                if (arg) {
                    arg->parent = refPtr;
                    refPtr->arguments.push_back(arg);
                }
                if (lex.current_token == token::comma) lex.advance();
            }
            if (lex.current_token == token::greater_than) lex.advance();
        }
        break;
    }
    case token::open_paren:
    {
        lex.advance();
        result = parse_type_reference(lex);
        if (lex.current_token == token::close_paren) lex.advance();
        break;
    }
    default:
        std::printf("ERROR: Unexpected token '%s' while parsing type\n", lex.string_value.c_str());
        return nullptr;
    }

    while (lex.current_token == token::open_bracket)
    {
        lex.advance();
        if (lex.current_token == token::close_bracket)
        {
            lex.advance();
            auto arr = std::make_unique<ast::array>();
            arr->type = result;
            result->parent = arr.get();
            result = arr.get();
            lex.file->nodes.push_back(std::move(arr));
        }
        else
        {
            while (lex.current_token != token::close_bracket && lex.current_token != token::eof) lex.advance();
            if (lex.current_token == token::close_bracket) lex.advance();
        }
    }

    if (lex.current_token == token::keyword_extends)
    {
        lex.advance();
        while (lex.current_token != token::question && lex.current_token != token::eof) lex.advance();
        if (lex.current_token == token::question) lex.advance();
        while (lex.current_token != token::colon && lex.current_token != token::eof) lex.advance();
        if (lex.current_token == token::colon) lex.advance();
        parse_single_type(lex);
        auto cond = std::make_unique<ast::conditional_type>();
        cond->condition = result;
        result = cond.get();
        lex.file->nodes.push_back(std::move(cond));
    }

    return result;
}

static ast::node* parse_type_reference(lexer& lex)
{
    std::vector<ast::node*> types;
    types.push_back(parse_single_type(lex));

    if (lex.current_token == token::pipe)
    {
        while (lex.current_token == token::pipe)
        {
            lex.advance();
            types.push_back(parse_single_type(lex));
        }
        auto un = std::make_unique<ast::union_type>();
        un->types = types;
        ast::node* result = un.get();
        lex.file->nodes.push_back(std::move(un));
        return result;
    }
    else if (lex.current_token == token::ampersand)
    {
        while (lex.current_token == token::ampersand)
        {
            lex.advance();
            types.push_back(parse_single_type(lex));
        }
        auto in = std::make_unique<ast::intersection_type>();
        in->types = types;
        ast::node* result = in.get();
        lex.file->nodes.push_back(std::move(in));
        return result;
    }

    return types[0];
}

static ast::type_alias* parse_type_alias(lexer& lex)
{
    assert(lex.current_token == token::keyword_type);
    lex.advance();

    if (lex.current_token != token::identifier)
    {
        std::printf("ERROR: Unexpected token '%s' for name of type alias; expected an identifier\n", lex.string_value.c_str());
        return nullptr;
    }

    auto result = std::make_unique<ast::type_alias>();
    result->name.swap(lex.string_value);

    lex.advance();

    if (lex.current_token == token::less_than)
    {
        while (lex.current_token != token::greater_than && lex.current_token != token::eof) lex.advance();
        if (lex.current_token == token::greater_than) lex.advance();
    }

    if (lex.current_token != token::equals)
    {
        std::printf("ERROR: Unexpected token '%s' after declaration of type alias '%s'; expected '='\n", lex.string_value.c_str(), result->name.c_str());
        return nullptr;
    }

    lex.advance();

    result->target_type = parse_type_reference(lex);
    if (!result->target_type)
    {
        std::printf("NOTE: While processing type alias '%s'\n", result->name.c_str());
        return nullptr;
    }
    result->target_type->parent = result.get();

    if (lex.current_token != token::semicolon)
    {
        std::printf("ERROR: Unexpected token '%s' after type alias '%s'; expected ';'\n", lex.string_value.c_str(), result->name.c_str());
        return nullptr;
    }
    lex.advance();

    auto resultPtr = result.get();
    lex.file->nodes.push_back(std::move(result));
    return resultPtr;
}

static ast::object* parse_object(lexer& lex)
{
    assert(lex.current_token == token::open_curly);
    lex.advance(); // Consume the '{'

    auto result = std::make_unique<ast::object>();
    while (lex.current_token != token::close_curly)
    {
        switch (lex.current_token)
        {
        case token::open_bracket:
        {
            lex.advance();
            auto member = std::make_unique<ast::member>();

            if (lex.current_token == token::identifier)
            {
                member->name.swap(lex.string_value);
                lex.advance();
            }

            if (lex.current_token == token::keyword_in)
            {
                lex.advance();
                parse_single_type(lex);
                if (lex.current_token == token::keyword_type || lex.current_token == token::identifier)
                {
                    lex.advance();
                }
            }
            else if (lex.current_token == token::colon)
            {
                lex.advance();
                parse_single_type(lex);
            }

            if (lex.current_token == token::close_bracket) lex.advance();

            if (lex.current_token == token::question)
            {
                member->is_optional = true;
                lex.advance();
            }

            if (lex.current_token != token::colon)
            {
                std::printf("ERROR: Unexpected token '%s' while parsing object member '%s'; expected ':'\n", lex.string_value.c_str(), member->name.c_str());
                return nullptr;
            }
            lex.advance();

            ast::node* type = parse_type_reference(lex);
            if (!type)
            {
                std::printf("NOTE: While processing object member '%s'\n", member->name.c_str());
                return nullptr;
            }

            member->type = type;
            type->parent = member.get();

            if (lex.current_token == token::semicolon || lex.current_token == token::comma)
            {
                lex.advance();
            }

            auto memberPtr = member.get();
            lex.file->nodes.push_back(std::move(member));
            result->named_members.push_back(memberPtr);
            break;
        }
        case token::keyword_module: // Allowed as an identifier in certain contexts
        case token::keyword_type:
        case token::keyword_keyof:
        case token::keyword_in:
        case token::keyword_unknown:
        case token::keyword_never:
        case token::identifier:
        {
            auto member = std::make_unique<ast::member>();
            member->name.swap(lex.string_value);
            lex.advance();

            if (lex.current_token == token::question)
            {
                member->is_optional = true;
                lex.advance();
            }

            if (lex.current_token != token::colon)
            {
                std::printf("ERROR: Unexpected token '%s' while parsing object member '%s'; expected ':'\n", lex.string_value.c_str(), member->name.c_str());
                return nullptr;
            }
            lex.advance();

            ast::node* type = parse_type_reference(lex);
            if (!type)
            {
                std::printf("NOTE: While processing object member '%s'\n", member->name.c_str());
                return nullptr;
            }

            if (lex.current_token == token::open_bracket)
            {
                auto arr = std::make_unique<ast::array>();
                arr->type = type;
                type->parent = arr.get();
                type = arr.get();
                lex.file->nodes.push_back(std::move(arr));

                lex.advance();
                if (lex.current_token != token::close_bracket)
                {
                    std::printf("ERROR: Unexpected token '%s' while parsing object member '%s'; expected ']'\n", lex.string_value.c_str(), member->name.c_str());
                    return nullptr;
                }
                lex.advance();
            }

            if (lex.current_token == token::semicolon || lex.current_token == token::comma)
            {
                lex.advance();
            }
            else if (lex.current_token != token::close_curly)
            {
                std::printf("ERROR: Unexpected token '%s' while parsing object member '%s'; expected ';', ',', or '}'\n", lex.string_value.c_str(), member->name.c_str());
                return nullptr;
            }

            member->type = type;
            type->parent = member.get();

            auto memberPtr = member.get();
            lex.file->nodes.push_back(std::move(member));
            result->named_members.push_back(memberPtr);
        }   break;

        default:
            std::printf("ERROR: Unexpected token '%s' while parsing object body\n", lex.string_value.c_str());
            return nullptr;
        }
    }

    lex.advance(); // Consume the '}'

    auto resultPtr = result.get();
    lex.file->nodes.push_back(std::move(result));
    return resultPtr;
}

static ast::interface* parse_interface(lexer& lex)
{
    assert(lex.current_token == token::keyword_interface);
    lex.advance();

    if (lex.current_token != token::identifier)
    {
        std::printf("ERROR: Unexpected token '%s' for name of interface; expected an identifier\n", lex.string_value.c_str());
        return nullptr;
    }

    auto result = std::make_unique<ast::interface>();
    result->name.swap(lex.string_value);

    lex.advance();
    if (lex.current_token == token::keyword_extends)
    {
        lex.advance();
        if (lex.current_token != token::identifier)
        {
            std::printf("ERROR: Unexpected token '%s' while parsing 'extends' type for interface '%s'; expected an identifier\n", lex.string_value.c_str(), result->name.c_str());
            return nullptr;
        }

        auto baseRef = std::make_unique<ast::interface_reference>();
        baseRef->name.swap(lex.string_value);
        result->base = baseRef.get();
        lex.file->nodes.push_back(std::move(baseRef));
        lex.advance();
    }

    if (lex.current_token != token::open_curly)
    {
        std::printf("ERROR: Unexpected token '%s' after declaration of interface '%s'; expected an '{'\n", lex.string_value.c_str(), result->name.c_str());
        return nullptr;
    }

    result->definition = parse_object(lex);
    if (!result->definition)
    {
        std::printf("NOTE: While processing interface '%s'\n", result->name.c_str());
        return nullptr;
    }
    result->definition->parent = result.get();

    auto resultPtr = result.get();
    lex.file->nodes.push_back(std::move(result));
    return resultPtr;
}

static ast::node* parse_export(lexer& lex)
{
    assert(lex.current_token == token::keyword_export);
    lex.advance();
    switch (lex.current_token)
    {
    case token::keyword_module:
    {
        auto result = parse_module(lex);
        if (result) result->is_export = true;
        return result;
    }

    case token::keyword_interface:
    {
        auto result = parse_interface(lex);
        if (result) result->is_export = true;
        return result;
    }

    case token::keyword_type:
    {
        auto result = parse_type_alias(lex);
        if (result) result->is_export = true;
        return result;
    }

    case token::keyword_enum:
    {
        auto result = parse_enum(lex);
        if (result) result->is_export = true;
        return result;
    }

    default:
        std::printf("ERROR: Unexpected token '%s' while parsing export\n", lex.string_value.c_str());
        return nullptr;
    }
}

static ast::import_stmt* parse_import(lexer& lex)
{
    assert(lex.current_token == token::keyword_import);
    lex.advance();

    while (lex.current_token != token::keyword_from && lex.current_token != token::string && lex.current_token != token::eof)
    {
        if (lex.current_token == token::open_curly) {
            lex.advance();
            while (lex.current_token != token::close_curly && lex.current_token != token::eof) lex.advance();
            if (lex.current_token == token::close_curly) lex.advance();
        } else {
            lex.advance();
        }
    }

    if (lex.current_token == token::keyword_from)
    {
        lex.advance();
    }

    if (lex.current_token != token::string)
    {
        std::printf("ERROR: Unexpected token '%s' while parsing import; expected a string\n", lex.string_value.c_str());
        return nullptr;
    }

    auto result = std::make_unique<ast::import_stmt>();
    result->module_name = lex.string_value;
    lex.advance();

    if (lex.current_token == token::semicolon)
    {
        lex.advance();
    }

    auto ptr = result.get();
    lex.file->nodes.push_back(std::move(result));
    return ptr;
}

static ast::enumeration* parse_enum(lexer& lex)
{
    assert(lex.current_token == token::keyword_enum);
    lex.advance();

    if (lex.current_token != token::identifier)
    {
        std::printf("ERROR: Unexpected token '%s' for name of enum; expected an identifier\n", lex.string_value.c_str());
        return nullptr;
    }

    auto result = std::make_unique<ast::enumeration>();
    result->name.swap(lex.string_value);

    lex.advance();
    if (lex.current_token != token::open_curly)
    {
        std::printf("ERROR: Unexpected token '%s' after declaration of enum '%s'; expected an '{'\n", lex.string_value.c_str(), result->name.c_str());
        return nullptr;
    }

    lex.advance();
    while (lex.current_token != token::close_curly && lex.current_token != token::eof)
    {
        if (lex.current_token == token::identifier)
        {
            ast::enum_member member;
            member.name = lex.string_value;
            lex.advance();
            if (lex.current_token == token::equals)
            {
                lex.advance();
                if (lex.current_token == token::string || lex.current_token == token::number_literal)
                {
                    member.value = lex.string_value;
                    lex.advance();
                }
            }
            result->members.push_back(member);
        }
        if (lex.current_token == token::comma)
        {
            lex.advance();
        }
    }

    if (lex.current_token == token::close_curly) lex.advance();

    auto resultPtr = result.get();
    lex.file->nodes.push_back(std::move(result));
    return resultPtr;
}

std::unique_ptr<ast::file> parse_file(std::istream& input)
{
    auto result = std::make_unique<ast::file>();

    lexer lex(input, result.get());
    bool firstToken = true;
    while (lex)
    {
        switch (lex.current_token)
        {
        case token::semicolon:
            lex.advance();
            break;

        case token::string:
            if (lex.string_value != "use strict"sv)
            {
                std::printf("ERROR: String '%s' unexpected at file scope\n", lex.string_value.c_str());
                return nullptr;
            }
            else if (lex.advance(); lex.current_token != token::semicolon)
            {
                std::printf("ERROR: Missing ';' after 'use strict'\n");
                return nullptr;
            }
            else if (!firstToken)
            {
                std::printf("ERROR: 'use strict' must be the first statement\n");
                return nullptr;
            }
            result->strict = true;
            lex.advance();
            break;

        case token::keyword_export:
        {
            auto ptr = parse_export(lex);
            if (!ptr) return nullptr;
            ptr->parent = result.get();
            result->children.push_back(ptr);
        }   break;

        case token::keyword_type:
        {
            auto ptr = parse_type_alias(lex);
            if (!ptr) return nullptr;
            ptr->parent = result.get();
            result->children.push_back(ptr);
        }   break;

        case token::keyword_import:
        {
            auto ptr = parse_import(lex);
            if (!ptr) return nullptr;
            ptr->parent = result.get();
            result->children.push_back(ptr);
        }   break;

        case token::keyword_interface:
        {
            auto ptr = parse_interface(lex);
            if (!ptr) return nullptr;
            ptr->parent = result.get();
            result->children.push_back(ptr);
        }   break;

        case token::keyword_enum:
        {
            auto ptr = parse_enum(lex);
            if (!ptr) return nullptr;
            ptr->parent = result.get();
            result->children.push_back(ptr);
        }   break;

        default:
            std::printf("ERROR: Token '%s' unexpected at file scope\n", lex.string_value.c_str());
            return nullptr;
        }

        firstToken = false;
    }

    return result;
}

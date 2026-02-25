#pragma once

#include <memory>
#include <string>
#include <vector>

namespace ast
{
    struct node
    {
        virtual ~node() {}

        node* parent = nullptr;
    };

    struct file : node
    {
        bool strict = false;
        std::vector<node*> children;

        // For cleanup
        std::vector<std::unique_ptr<node>> nodes;
    };

    struct import_stmt : node
    {
        std::string module_name;
    };

    struct module : node
    {
        bool is_export = false;
        std::string name;
        std::vector<node*> children;
    };

    struct member : node
    {
        bool is_optional = false;
        std::string name;
        node* type;
    };

    struct object : node
    {
        std::vector<member*> named_members;
        // TODO: unnamed members (i.e. arbitrary key:value pairs)
    };

    struct interface : node
    {
        bool is_export = false;
        std::vector<node*> base;
        std::string name;
        object* definition = nullptr;
    };

    struct interface_reference : node
    {
        std::string name;
    };

    enum class fundamental_type
    {
        any,
        boolean,
        number,
        string,
        unknown,
        never,
    };

    struct fundamental_type_reference : node
    {
        fundamental_type type;
        fundamental_type_reference(fundamental_type type) : type(type) {}
    };

    struct array : node
    {
        node* type;
    };

    struct enum_member {
        std::string name;
        std::string value;
    };

    struct enumeration : node
    {
        bool is_export = false;
        std::string name;
        std::vector<enum_member> members;
    };

    struct type_alias : node
    {
        bool is_export = false;
        std::string name;
        node* target_type = nullptr;
    };

    struct union_type : node
    {
        std::vector<node*> types;
    };

    struct intersection_type : node
    {
        std::vector<node*> types;
    };

    struct literal_type : node
    {
        std::string value;
        bool is_string = false;
        bool is_number = false;
    };

    struct tuple_type : node
    {
        std::vector<node*> elements;
    };

    struct template_literal_type : node
    {
        std::string value;
    };

    struct generic_type_reference : node
    {
        std::string name;
        std::vector<node*> arguments;
    };

    struct mapped_type : node
    {
        node* key_type = nullptr;
        node* value_type = nullptr;
    };

    struct conditional_type : node
    {
        node* condition = nullptr;
        node* extends_type = nullptr;
        node* true_type = nullptr;
        node* false_type = nullptr;
    };
}

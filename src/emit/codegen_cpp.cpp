#include "codegen_cpp.h"
#include <ostream>
#include <sstream>
#include <set>

struct codegen_state
{
    std::stringstream out;
    const codegen_config& config;
    std::set<std::string> headers;
    std::map<std::string, ast::node*> known_nodes;

    codegen_state(const codegen_config& conf) : config(conf) {}

    void add_header(const std::string& h)
    {
        if (!h.empty()) {
            headers.insert(h);
        }
    }
};

static void generate_type(codegen_state& state, ast::node* type);

static void generate_module(codegen_state& state, ast::module* mod)
{
    state.out << "namespace " << mod->name << " {\n";
    for (auto* child : mod->children)
    {
        generate_type(state, child);
    }
    state.out << "} // namespace " << mod->name << "\n\n";
}

static void generate_object_body(codegen_state& state, ast::object* obj)
{
    state.out << "{\n";
    for (auto* member : obj->named_members)
    {
        std::string type_str;
        codegen_state temp_state(state.config);
        temp_state.known_nodes = state.known_nodes;
        generate_type(temp_state, member->type);
        type_str = temp_state.out.str();

        if (type_str.find("std::any /*") != std::string::npos &&
            type_str != "std::any /* unknown */" &&
            type_str != "std::any /* never */") {
            continue;
        }

        state.out << "    ";
        if (member->is_optional) {
            state.add_header("#include <optional>");
            state.out << "std::optional<" << type_str << ">";
        } else {
            state.out << type_str;
        }
        for (const auto& h : temp_state.headers) state.add_header(h);
        state.out << " " << member->name << ";\n";
    }
    state.out << "}";
}

static void generate_interface(codegen_state& state, ast::interface* iface)
{
    state.out << "struct " << iface->name;
    if (!iface->base.empty())
    {
        state.out << " : ";
        for (size_t i = 0; i < iface->base.size(); ++i)
        {
            if (i > 0) state.out << ", ";
            state.out << "public ";
            generate_type(state, iface->base[i]);
        }
    }
    state.out << "\n";
    if (iface->definition)
    {
        generate_object_body(state, iface->definition);
        state.out << ";\n\n";
    }
    else
    {
        state.out << "{};\n\n";
    }
}

static bool collect_members(codegen_state& state, ast::node* type, std::vector<ast::member*>& members)
{
    if (!type) return false;
    if (auto* gref = dynamic_cast<ast::generic_type_reference*>(type)) {
        auto it = state.known_nodes.find(gref->name);
        if (it != state.known_nodes.end()) {
            return collect_members(state, it->second, members);
        }
        return false;
    } else if (auto* iface = dynamic_cast<ast::interface*>(type)) {
        for (auto* b : iface->base) {
            if (!collect_members(state, b, members)) return false;
        }
        if (iface->definition) {
            for (auto* m : iface->definition->named_members) members.push_back(m);
        }
        return true;
    } else if (auto* alias = dynamic_cast<ast::type_alias*>(type)) {
        return collect_members(state, alias->target_type, members);
    } else if (auto* obj = dynamic_cast<ast::object*>(type)) {
        for (auto* m : obj->named_members) members.push_back(m);
        return true;
    } else if (auto* in = dynamic_cast<ast::intersection_type*>(type)) {
        for (auto* t : in->types) {
            if (!collect_members(state, t, members)) return false;
        }
        return true;
    }
    return false;
}

static std::string make_identifier(const std::string& str) {
    if (str.empty()) return "_";
    std::string res = str;
    for (char& c : res) {
        if (!std::isalnum(c)) c = '_';
    }
    if (std::isdigit(res[0])) res = "_" + res;
    return res;
}

static bool is_literal_union_or_single(codegen_state& state, ast::node* node, std::vector<std::string>& values) {
    if (!node) return false;
    if (auto* lit = dynamic_cast<ast::literal_type*>(node)) {
        values.push_back(lit->value);
        return true;
    } else if (auto* un = dynamic_cast<ast::union_type*>(node)) {
        for (auto* t : un->types) {
            if (!is_literal_union_or_single(state, t, values)) return false;
        }
        return true;
    } else if (auto* gref = dynamic_cast<ast::generic_type_reference*>(node)) {
        if (gref->name == "Capitalize" || gref->name == "Uncapitalize" || gref->name == "Uppercase" || gref->name == "Lowercase") {
            if (!gref->arguments.empty()) {
                std::vector<std::string> base_vals;
                if (is_literal_union_or_single(state, gref->arguments[0], base_vals)) {
                    for (auto& v : base_vals) {
                        if (v.empty()) continue;
                        if (gref->name == "Capitalize") {
                            v[0] = std::toupper(v[0]);
                        } else if (gref->name == "Uncapitalize") {
                            v[0] = std::tolower(v[0]);
                        } else if (gref->name == "Uppercase") {
                            for (char& c : v) c = std::toupper(c);
                        } else if (gref->name == "Lowercase") {
                            for (char& c : v) c = std::tolower(c);
                        }
                        values.push_back(v);
                    }
                    return true;
                }
            }
        } else if (gref->name == "Exclude" || gref->name == "Extract") {
            if (gref->arguments.size() >= 2) {
                std::vector<std::string> base_vals, filter_vals;
                if (is_literal_union_or_single(state, gref->arguments[0], base_vals) && is_literal_union_or_single(state, gref->arguments[1], filter_vals)) {
                    std::set<std::string> filter_set(filter_vals.begin(), filter_vals.end());
                    for (auto& v : base_vals) {
                        bool present = filter_set.count(v);
                        if (gref->name == "Exclude" && !present) values.push_back(v);
                        if (gref->name == "Extract" && present) values.push_back(v);
                    }
                    return true;
                }
            }
        } else {
            auto it = state.known_nodes.find(gref->name);
            if (it != state.known_nodes.end()) {
                if (auto* alias = dynamic_cast<ast::type_alias*>(it->second)) {
                    return is_literal_union_or_single(state, alias->target_type, values);
                } else if (auto* en = dynamic_cast<ast::enumeration*>(it->second)) {
                    for (auto& m : en->members) {
                        std::string val = m.value;
                        if (val.empty()) val = m.name;
                        if (val.size() >= 2 && val.front() == '"' && val.back() == '"') {
                            val = val.substr(1, val.size() - 2);
                        }
                        values.push_back(val);
                    }
                    return true;
                }
            }
        }
    }
    return false;
}

static void generate_type_alias(codegen_state& state, ast::type_alias* alias)
{
    if (auto* in = dynamic_cast<ast::intersection_type*>(alias->target_type)) {
        std::vector<ast::member*> members;
        bool all_known = true;
        for (auto* t : in->types) {
            if (!collect_members(state, t, members)) {
                all_known = false;
                break;
            }
        }
        if (all_known) {
            state.out << "struct " << alias->name << "\n{\n";
            for (auto* member : members)
            {
                state.out << "    ";
                if (member->is_optional) {
                    state.add_header("#include <optional>");
                    state.out << "std::optional<";
                }
                generate_type(state, member->type);
                if (member->is_optional) state.out << ">";
                state.out << " " << member->name << ";\n";
            }
            state.out << "};\n\n";
        }
        return;
    }

    if (auto* gref = dynamic_cast<ast::generic_type_reference*>(alias->target_type)) {
        if (gref->name == "Partial" || gref->name == "Readonly" || gref->name == "Omit" || gref->name == "Pick" || gref->name == "NonNullable") {
            if (!gref->arguments.empty()) {
                std::vector<ast::member*> members;
                if (collect_members(state, gref->arguments[0], members)) {
                    std::set<std::string> omitted;
                    if ((gref->name == "Omit" || gref->name == "Pick") && gref->arguments.size() > 1) {
                        std::vector<std::string> omit_keys;
                        is_literal_union_or_single(state, gref->arguments[1], omit_keys);
                        for (const auto& k : omit_keys) {
                            std::string ck = k;
                            if (ck.size() >= 2 && ck.front() == '"' && ck.back() == '"') ck = ck.substr(1, ck.size() - 2);
                            omitted.insert(ck);
                        }
                    }

                    state.out << "struct " << alias->name << "\n{\n";
                    for (auto* m : members) {
                        if (gref->name == "Omit" && omitted.count(m->name)) continue;
                        if (gref->name == "Pick" && !omitted.count(m->name)) continue;

                        state.out << "    ";
                        bool is_opt = m->is_optional;
                        if (gref->name == "Partial") is_opt = true;

                        if (gref->name == "Readonly") state.out << "const ";

                        if (is_opt) {
                            state.add_header("#include <optional>");
                            state.out << "std::optional<";
                        }

                        generate_type(state, m->type);

                        if (is_opt) state.out << ">";

                        state.out << " " << m->name << ";\n";
                    }
                    state.out << "};\n\n";
                    return;
                }
            }
        }
    }

    std::vector<std::string> literal_values;
    if (is_literal_union_or_single(state, alias->target_type, literal_values)) {
        state.out << "enum class " << alias->name << " {\n";
        for (const auto& val : literal_values) {
            state.out << "    " << make_identifier(val) << ",\n";
        }
        state.out << "};\n\n";
        if (state.config.cpp.enum_mode == enum_generation_mode::with_array) {
            state.out << "constexpr const char* " << alias->name << "Strings[] = {\n";
            for (const auto& val : literal_values) {
                state.out << "    \"" << val << "\",\n";
            }
            state.out << "};\n\n";
        }
        return;
    }

    if (auto* obj = dynamic_cast<ast::object*>(alias->target_type)) {
        state.out << "struct " << alias->name << "\n";
        generate_object_body(state, obj);
        state.out << ";\n\n";
        return;
    }

    std::string type_str;
    {
        codegen_state temp_state(state.config);
        temp_state.known_nodes = state.known_nodes;
        generate_type(temp_state, alias->target_type);
        type_str = temp_state.out.str();

        if (type_str.find("std::any /*") != std::string::npos &&
            type_str != "std::any /* unknown */" &&
            type_str != "std::any /* never */") {
            return;
        }
        for (const auto& h : temp_state.headers) state.add_header(h);
    }
    state.out << "using " << alias->name << " = " << type_str << ";\n\n";
}

static void generate_enum(codegen_state& state, ast::enumeration* en)
{
    state.out << "enum class " << en->name << " {\n";
    for (const auto& member : en->members)
    {
        state.out << "    " << make_identifier(member.name);
        state.out << ",\n";
    }
    state.out << "};\n\n";

    if (state.config.cpp.enum_mode == enum_generation_mode::with_array) {
        state.out << "constexpr const char* " << en->name << "Strings[] = {\n";
        for (const auto& member : en->members) {
            std::string val = member.value;
            if (val.empty()) val = member.name;
            else if (val.size() >= 2 && val.front() == '"' && val.back() == '"') val = val.substr(1, val.size() - 2);
            state.out << "    \"" << val << "\",\n";
        }
        state.out << "};\n\n";
    }
}

static void check_config(codegen_state& state, const std::string& type_name, const std::string& fallback, const std::string& fallback_header = "")
{
    auto it = state.config.datatypes.find(type_name);
    if (it != state.config.datatypes.end())
    {
        state.out << it->second.out;
        state.add_header(it->second.header);
    }
    else
    {
        state.out << fallback;
        state.add_header(fallback_header);
    }
}

static void generate_import(codegen_state& state, ast::import_stmt* imp)
{
    state.add_header("#include \"" + imp->module_name + ".h\"");
}

static void generate_type(codegen_state& state, ast::node* type)
{
    if (!type) { check_config(state, "any", "std::any", "#include <any>"); return; }

    if (auto* mod = dynamic_cast<ast::module*>(type))
    {
        generate_module(state, mod);
    }
    else if (auto* imp = dynamic_cast<ast::import_stmt*>(type))
    {
        generate_import(state, imp);
    }
    else if (auto* iface = dynamic_cast<ast::interface*>(type))
    {
        generate_interface(state, iface);
    }
    else if (auto* alias = dynamic_cast<ast::type_alias*>(type))
    {
        generate_type_alias(state, alias);
    }
    else if (auto* en = dynamic_cast<ast::enumeration*>(type))
    {
        generate_enum(state, en);
    }
    else if (auto* ref = dynamic_cast<ast::interface_reference*>(type))
    {
        check_config(state, ref->name, ref->name, "");
    }
    else if (auto* gref = dynamic_cast<ast::generic_type_reference*>(type))
    {
        if (gref->name == "undefined") {
            state.add_header("#include <variant>");
            state.out << "std::monostate";
            return;
        } else if (gref->name == "Array" || gref->name == "ReadonlyArray") {
            state.add_header("#include <vector>");
            state.out << "std::vector<";
            if (!gref->arguments.empty()) generate_type(state, gref->arguments[0]);
            else state.out << "std::any";
            state.out << ">";
        } else if (gref->name == "Record") {
            state.add_header("#include <map>");
            state.out << "std::map<";
            if (gref->arguments.size() >= 1) generate_type(state, gref->arguments[0]); else state.out << "std::string";
            state.out << ", ";
            if (gref->arguments.size() >= 2) generate_type(state, gref->arguments[1]); else state.out << "std::any";
            state.out << ">";
        } else if (gref->name == "NonNullable") {
            if (!gref->arguments.empty()) generate_type(state, gref->arguments[0]);
            else {
                 state.add_header("#include <any>");
                 state.out << "std::any /* NonNullable */";
            }
        } else if (gref->name == "Partial" || gref->name == "Readonly" || gref->name == "Omit" || gref->name == "Pick" || gref->name == "Capitalize" || gref->name == "Uncapitalize" || gref->name == "Uppercase" || gref->name == "Lowercase" || gref->name == "Exclude" || gref->name == "Extract") {
            // Fallback for unresolved utility types
            state.add_header("#include <any>");
            state.out << "std::any /* " << gref->name << " */";
        } else {
            check_config(state, gref->name, gref->name, "");
            if (!gref->arguments.empty()) {
                state.out << "<";
                for (size_t i = 0; i < gref->arguments.size(); ++i) {
                    if (i > 0) state.out << ", ";
                    generate_type(state, gref->arguments[i]);
                }
                state.out << ">";
            }
        }
    }
    else if (auto* f = dynamic_cast<ast::fundamental_type_reference*>(type))
    {
        switch (f->type)
        {
        case ast::fundamental_type::any: check_config(state, "any", "std::any", "#include <any>"); break;
        case ast::fundamental_type::boolean: check_config(state, "boolean", "bool"); break;
        case ast::fundamental_type::number: check_config(state, "number", "double"); break;
        case ast::fundamental_type::string: check_config(state, "string", "std::string", "#include <string>"); break;
        case ast::fundamental_type::unknown: check_config(state, "unknown", "std::any /* unknown */", "#include <any>"); break;
        case ast::fundamental_type::never: check_config(state, "never", "std::any /* never */", "#include <any>"); break;
        }
    }
    else if (auto* arr = dynamic_cast<ast::array*>(type))
    {
        state.add_header("#include <vector>");
        state.out << "std::vector<";
        generate_type(state, arr->type);
        state.out << ">";
    }
    else if (auto* un = dynamic_cast<ast::union_type*>(type))
    {
        state.add_header("#include <variant>");
        state.out << "std::variant<";
        for (size_t i = 0; i < un->types.size(); ++i)
        {
            if (i > 0) state.out << ", ";
            generate_type(state, un->types[i]);
        }
        state.out << ">";
    }
    else if (auto* in = dynamic_cast<ast::intersection_type*>(type))
    {
        // Intersections inline that are unknown or have no name output nothing or std::any.
        // Handled completely in type_aliases if named.
        state.add_header("#include <any>");
        state.out << "std::any /* inline intersection */";
    }
    else if (auto* tup = dynamic_cast<ast::tuple_type*>(type))
    {
        state.add_header("#include <tuple>");
        state.out << "std::tuple<";
        for (size_t i = 0; i < tup->elements.size(); ++i)
        {
            if (i > 0) state.out << ", ";
            generate_type(state, tup->elements[i]);
        }
        state.out << ">";
    }
    else if (auto* lit = dynamic_cast<ast::literal_type*>(type))
    {
        if (lit->is_string) {
            check_config(state, "string", "std::string", "#include <string>");
            state.out << " /* " << lit->value << " */";
        } else if (lit->is_number) {
            check_config(state, "number", "double");
            state.out << " /* " << lit->value << " */";
        } else {
            check_config(state, "any", "std::any", "#include <any>");
            state.out << " /* literal */";
        }
    }
    else if (auto* obj = dynamic_cast<ast::object*>(type))
    {
        state.add_header("#include <map>");
        state.add_header("#include <string>");
        state.add_header("#include <any>");
        state.out << "std::map<std::string, std::any> /* object */";
    }
    else if (auto* ct = dynamic_cast<ast::conditional_type*>(type))
    {
        state.add_header("#include <any>");
        state.out << "std::any /* conditional */";
    }
    else
    {
        state.add_header("#include <any>");
        state.out << "std::any /* unmapped type */";
    }
}

void generate_cpp(std::ostream& out, ast::file* file, const codegen_config& config)
{
    codegen_state state(config);

    for (auto* child : file->children)
    {
        if (auto* iface = dynamic_cast<ast::interface*>(child)) {
            state.known_nodes[iface->name] = iface;
        } else if (auto* alias = dynamic_cast<ast::type_alias*>(child)) {
            state.known_nodes[alias->name] = alias;
        } else if (auto* en = dynamic_cast<ast::enumeration*>(child)) {
            state.known_nodes[en->name] = en;
        }
    }

    for (auto* child : file->children)
    {
        generate_type(state, child);
    }

    out << "// Auto-generated by ts-type-conv\n"
        << "#pragma once\n\n";

    for (const auto& h : state.headers)
    {
        out << h << "\n";
    }
    if (!state.headers.empty()) out << "\n";

    out << state.out.str();
}

### Boost.JSON tag_invoke Support

Add functionality to generate `boost::json::tag_invoke` overloads for generated C++ structs. This will enable seamless integration with `boost::json::value_from` and `boost::json::value_to`, allowing for automated serialization and deserialization between JSON and the transpiled C++ types.

* **Objective**: Eliminate manual boilerplate for JSON conversion in the target C++ code.
* **Implementation**: For each generated struct, generate the corresponding `tag_invoke` functions within the same namespace (or as friends) to satisfy the Boost.JSON library requirements.

### Magic Enum Integration

Add functionality to integrate `magic_enum` for generated C++ enumerations. This will provide static reflection for enums, allowing for easy conversion between enum values and strings without manual switch statements or mapping tables.

* **Objective**: Provide string conversion and reflection capabilities for TypeScript enums transpiled to C++.
* **Implementation**: For each generated enum, include the `magic_enum.hpp` header and ensure the enum definitions are compatible with `magic_enum`'s requirements (e.g., supported range limits) to enable functions like `magic_enum::enum_name` and `magic_enum::enum_cast`.

### Switch Lexer/Parser to Flex++ and Bison++

Evaluate switching the current hand-written lexer and recursive-descent parser to use Flex++ (for lexical analysis) and Bison++ (for parser generation).

* **Objective**: Replace the manual parsing logic with industry-standard tooling constructed from formal context-free grammar constraints, enabling faster iterations on complex TypeScript definitions.
* **Implementation**: Introduce `.l` and `.y` files defining the TypeScript lexical boundaries and grammar rules, modifying `CMakeLists.txt` to fetch or utilize `Flex`/`Bison` generation steps prior to compiling the C++ executable. Wrap outputs elegantly to construct the existing `ast::node` architecture.
* **Effort Estimate**: High (~3-4 days). While rewriting the lexer (`.l`) is straightforward (Low-Medium effort), rewriting the parser (`.y`) to construct the AST requires a complete translation of the TS grammar subset. Furthermore, cross-platform build configurations (CMake/MSVC) will require significant adjustments to seamlessly support `winflexbison`.

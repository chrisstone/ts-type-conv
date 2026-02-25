# Supported TypeScript Features

The `ts-type-conv` tool parses a modern subset of TypeScript and maps it to C++ constructs and Protobuf definitions.

## Supported Constructs
- **Interfaces**: Mapped to C++ `struct`s. Supports inheritance (`extends`).
- **Type Aliases**: Mapped to C++ `using` declarations.
- **Enums**: Mapped to C++ `enum class`.
- **Arrays**: `T[]`, `Array<T>`, and `ReadonlyArray<T>` are mapped to `std::vector<T>`.
- **Tuples**: `[A, B, C]` mapped to `std::tuple<A, B, C>`.
- **Unions**: `A | B` mapped to `std::variant<A, B>`. (C++ Only - not natively supported in Proto).
- **Imports**: `import { Type } from 'module'` mapped to `#include "module.h"` or `import "module.proto"`.
- **Optional Members**: `foo?: string` mapped to `std::optional<string>` (or the specified optional wrap) or `optional string`.
- **Undefined**: `undefined` maps safely to `std::monostate`, rendering as `std::variant<T, std::monostate>` when paired with a type in C++.
- **Literal Types**: String and number literals are parsed and emitted as their parent config types (e.g. `std::string`) with a trailing comment identifying the original literal. Explicit inline union literals (`"a" | "b"`) natively convert into corresponding C++ or Proto Enums.
- **Intersection Types**: Support for inline recursive intersections. Named intersections structurally unwind into new inline `struct` / `message` members uniting all intersecting values.

## Supported Utility Types
- **Property Modifiers**: `Partial<T>`, `Readonly<T>`, `Omit<T, K>`, `Pick<T, K>`, and `NonNullable<T>` are intrinsically unwound and correctly emit C++ `struct` definitions identically modeling their logical configurations (e.g., dropping struct properties for `Omit`, emitting `std::optional` wraps for `Partial` props, etc.).
- **Mapped Records**: `Record<K, V>` organically generates an exact standard `std::map<K, V>`.
- **String Manipulators**: Enclosing string union literals in `Capitalize`, `Uncapitalize`, `Uppercase`, and `Lowercase` safely apply their C++ string-equivalent modifications onto the values generated.
- **Type Filtering**: `Exclude<T, U>` and `Extract<T, U>` statically compute intersecting values dynamically comparing subsets of static/union literals logic correctly.

## Unsupported or Vague Constructs
As listed in [todo.md](todo.md), specialized generative concepts (like runtime template string interpolants `something_${string}`, dynamic generic Maps, Conditional Types, and functional methods like `Awaited`, `ReturnType`) do not strictly align to logical pre-compiled data-type bounds and will consequently output as `std::any`, bypass generation entirely, or evaluate incorrectly on assumption.

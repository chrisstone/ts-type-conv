# TOML Configuration

The optional TOML configuration file allows you to customize how ts-type-conv maps TypeScript types to C++ or Proto types. When provided as the third argument, it overrides the built-in defaults for any type you specify.

```
codegen <input> <output> [config.toml]
```

## `format`

A top-level string that selects the output language. Currently `"cpp"` and `"proto"` are supported. The default is `"cpp"` when omitted.

```toml
format = "proto"
```

## `[cpp]` (C++ Generation Only)

Configuration block for specific C++ language generation mechanics.

| Key      | Type   | Description |
|----------|--------|-------------|
| `enum` | string | Providing `"withArray"` will forcefully generate a `constexpr const char* NameStrings[]` mapped array alongside every enum that matches its keys in parallel. Defaults to `"standard"`. |

### Example

```toml
[cpp]
enum = "withArray"
```

## `[datatype.<name>]`

Each table under `datatype` overrides how a particular TypeScript type is emitted in the generated code.

| Key      | Type   | Description |
|----------|--------|-------------|
| `out`    | string | The C++ or Protobuf type string to emit (e.g. `"int32_t"` or `"int32"`) |
| `header` | string | An `#include` directive to add to the generated header (C++ only) |

Both keys are optional within a table entry. If `header` is provided, the include directive is automatically added to the top of the generated file.

### Example

```toml
[datatype.number]
out = "int32_t"
header = "#include <cstdint>"
```

This will cause every TypeScript `number` to be emitted as `int32_t` instead of the default `double`, and `#include <cstdint>` will appear in the generated header.

## Built-in Defaults

The following table shows all fundamental TypeScript types that can be overridden, along with their default C++ mappings:

| TypeScript Type | Default C++ Type | Default Header |
|-----------------|------------------|----------------|
| `any`           | `std::any`       | `#include <any>` |
| `boolean`       | `bool`           | _(none)_ |
| `number`        | `double`         | _(none)_ |
| `string`        | `std::string`    | `#include <string>` |
| `unknown`       | `std::any`       | `#include <any>` |
| `never`         | `std::any`       | `#include <any>` |

## Overriding Named Types

In addition to fundamental types, you can override any **interface reference** or **generic type reference** by name. For example, if your TypeScript uses `Record<string, any>`:

```toml
[datatype.Record]
out = "std::unordered_map<std::string, std::any>"
header = "#include <unordered_map>"
```

This causes any reference to `Record` to emit `std::unordered_map<std::string, std::any>` (or custom proto string) in the generated output.

## Complete Example

```toml
format = "cpp"

[cpp]
enum = "withArray"


[datatype.number]
out = "int32_t"
header = "#include <cstdint>"

[datatype.string]
out = "std::string_view"
header = "#include <string_view>"

[datatype.boolean]
out = "bool"

[datatype.Record]
out = "std::unordered_map<std::string, std::any>"
header = "#include <unordered_map>"

[datatype.Promise]
out = "std::future"
header = "#include <future>"
```

# TypeScript to C++ and Protobuf (ts-type-conv)
The primary purpose of this project is to convert interfaces defined in TypeScript into C++ type definitions or Protobuf schemas. For example, when interfacing with a protocol that returns JSON whose documentation/specification is written in TypeScript. In addition to converting types, it will also generate "reflection" structures/functions that are used when serializing/parsing JSON text.

## Examples
Here are a few examples that describe how the conversion process works

### Simple Example
Take the following TypeScript:
```ts
export module Foo {
    export interface Bar {
        x: number;
    }
}
```
This will generate the following type definition in C++:
```c++
namespace Foo
{
    struct Bar
    {
        double x;
    };
}
```

### Optional Example
Optional members get converted as the `json::optional_t<>` type (which is just a typedef for `std::optional<>`). E.g.:
```ts
export interface LoginInfo {
    username: string;
    password?: string;
}
```
Will become:
```c++
struct LoginInfo
{
    std::string username;
    std::optional<std::string> password;
};
```

### Unnamed Structure Example
If the type of a member is an object declared inline (i.e. using the `foo: { ... }` syntax), then the generated structure name will attempt to combine the member and interface name. For example:
```ts
export interface Computer {
    screen: {
        width: number;
        height: number;
    }
}
```
Will become:
```c++
struct ComputerScreen
{
    double width;
    double height;
};

struct Computer
{
    ComputerScreen screen;
};
```

### Enumeration Example
Enumerations are extracted out and named in an identical manner to unnamed structures. For example, take the following:
```ts
export interface Car {
    make: 'Ford' | 'Honda' | 'Nissan';
}
```
Will become:
```c++
enum class CarMake
{
    Ford,
    Honda,
    Nissan,
};

struct Car
{
    CarMake make;
};
```
If the enumeration is a member of an unnamed structure, the interface name used to generate the enum name will be the first user-supplied interface name. E.g. if we modify the above:
```ts
export interface Car {
    info: {
        make: 'Ford' | 'Honda' | 'Nissan';
    };
}
```
The generated enum will still be named `CarMake`.

## Usage
```
ts-type-conv <input_file | -> <output_file | -> [config.toml]
```

| Argument | Description |
|----------|-------------|
| `input_file` | Path to a TypeScript file, or `-` to read from stdin |
| `output_file` | Path for the generated output, or `-` to write to stdout |
| `config.toml` | Optional path to a TOML configuration file |

## Documentation
Detailed documentation is stored in the `doc/` directory:

- [Features (doc/features.md)](doc/features.md) - A full list of supported TypeScript syntax mappings.
- [Configuration (doc/configuration.md)](doc/configuration.md) - Learn how to provide a mapped custom TOML configuration to override TypeScript types globally.
- [TODOs (doc/todo.md)](doc/todo.md) - Discover parsing and generation limitations with C++ representations for structurally dynamic logic bounds and functionally complex concepts.

## Building
The project uses CMake and requires C++17. On Windows with MSVC Build Tools:

```bat
build.bat
```

Or manually:
```bash
mkdir build && cd build
cmake ..
cmake --build .
ctest -C Debug --output-on-failure
```

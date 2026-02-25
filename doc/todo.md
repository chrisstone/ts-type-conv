# TypeScript to C++ / Protobuf Generation TODOs

During the generation of C++ or Proto code, certain TypeScript concepts are non-trivial to represent in target implementations directly and are therefore currently outputted as `std::any` (or bypassed). This file tracks the types and structures that may require reevaluation or a more structured code generation approach in the future.

## Fundamental / Abstract Types
- `unknown`: Output as `std::any /* unknown */`.
- `never`: Output as `std::any /* never */`.
- `Conditional Types` (`T extends U ? X : Y`): Output as `std::any /* conditional */` as they are difficult to conditionally resolve at generation time without a full TypeScript type checker.
- Unmapped or recognized generic types without configurations output their name, which may cause compilation errors if not defined.

## Object Maps and Intersections
- Inline object definitions inside types without explicit fields context are mapped globally to `std::map<std::string, std::any> /* object */`.
- `Intersection Types` (`A & B`): If members are known, a new struct is emitted with the intersecting members. If the members are not known, the invalid type is not output and is bypassed.

## Mapped Types
- Mapped Types such as `[K in keyof Flags]?: boolean;` (e.g. `FeatureFlags`) are currently output as a generic `std::map<std::string, std::any> /* object */`. They should eventually be treated similar to `Partial<Flags>` and emit a struct with corresponding fields modified if the target structure is known.

## Intentionally Unsupported
The following keywords / utility types are currently not supported and bypass emission, primarily because they heavily assume logical functional contexts over strict data-type definitions:
- **Function Utility Types**: `Parameters`, `ConstructorParameters`, `ReturnType`, `InstanceType`, `NoInfer`, `ThisParameterType`, `OmitThisParameter`, `ThisType`
- **Dynamic Type Parsing**: `typeof`, `Awaited`
- **Template Literal Interpolations**: Strings with inline templates that do not cleanly isolate to a structural generic (e.g. `` `event_${name}` ``).
- **TS Unions**: Logical TS Unions (`A | B`) map effectively into `std::variant<A, B>` in C++, but currently do not map effectively into Protobuf representations (`oneof`) natively in this tool yet.

# Error handling and exceptions

yaml-cpp reports failures by throwing C++ exceptions. The types are declared in [`include/yaml-cpp/exceptions.h`](../include/yaml-cpp/exceptions.h). All public exceptions derive from `YAML::Exception` (which derives from `std::runtime_error`) and expose a `mark` member when the error is tied to a location in the input document.

This page describes the exceptions you can rely on when using the **current node API** (`YAML::Load`, `YAML::Node`, `.as<T>()`, etc.). Catching `YAML::Exception` is sufficient for most applications.

## Parsing and I/O

| Exception | When it is thrown |
|-----------|-------------------|
| `ParserException` | The input is not valid YAML (syntax, structure, directives, etc.) during `YAML::Load`, `YAML::LoadFile`, or streaming parse. `what()` includes line/column when `mark` is set. |
| `BadFile` | `YAML::LoadFile` cannot open or read the given path. |

Example:

```cpp
try {
  YAML::Node doc = YAML::LoadFile("config.yaml");
} catch (const YAML::ParserException& e) {
  // malformed YAML — e.mark.line / e.mark.column are 0-based
  std::cerr << e.what() << "\n";
} catch (const YAML::BadFile& e) {
  std::cerr << "cannot read file: " << e.what() << "\n";
}
```

## Optional map keys (no exception)

`operator[]` on a map **does not** throw when a key is missing. It returns a node that evaluates to false in a boolean context (see the [Tutorial](Tutorial.md)). Use that for optional fields:

```cpp
YAML::Node config = YAML::LoadFile("config.yaml");

if (config["lastLogin"]) {
  auto t = config["lastLogin"].as<std::string>();
}

// This does NOT throw KeyNotFound:
YAML::Node missing = config["notpresentkey"];
```

`YAML::KeyNotFound` is defined in the public headers for historical/API reasons, but the current node implementation does **not** throw it for missing map keys. Prefer `if (node["key"])` (or `node[key]` with a defined node) before calling `.as<T>()`.

## Type conversion and node misuse

| Exception | When it is thrown |
|-----------|-------------------|
| `BadConversion` | Generic conversion failure from `.as<T>()`. |
| `TypedBadConversion<T>` | Same as `BadConversion`, thrown by `.as<T>()` when the scalar cannot be converted to `T` (wrong type, out of range, etc.). |
| `InvalidNode` | Using a node incorrectly: e.g. `.as<T>()` on an undefined node, wrong iterator use, invalid key path. Message may include the first invalid key when available. |
| `BadSubscript` | `operator[]` used on a **scalar** node (not a map/sequence). |
| `BadPushback` | `push_back` on a non-sequence node. |
| `BadInsert` | `insert` on a node that cannot behave as a map. |
| `BadDereference` | Dereferencing an invalid node (rare in normal use). |
| `NonUniqueMapKey` | Inserting a duplicate key into a map node. |

Example (user-provided config with wrong types):

```cpp
try {
  YAML::Node doc = YAML::LoadFile("user.yaml");
  if (!doc["port"]) {
    throw std::runtime_error("missing required key: port");
  }
  unsigned port = doc["port"].as<unsigned>();
} catch (const YAML::TypedBadConversion<unsigned>& e) {
  std::cerr << "port is not a valid unsigned integer: " << e.what() << "\n";
} catch (const YAML::RepresentationException& e) {
  std::cerr << "config node error: " << e.what() << "\n";
} catch (const YAML::ParserException& e) {
  std::cerr << "invalid YAML: " << e.what() << "\n";
}
```

`RepresentationException` is the common base for conversion and node-operation errors listed above (except `ParserException` and `BadFile`).

## Emitting YAML

| Exception | When it is thrown |
|-----------|-------------------|
| `EmitterException` | Invalid emitter state or output (e.g. via `YAML::Emitter` / stream output). |

## Internal errors

Some code paths may throw `std::runtime_error` with messages like `yaml-cpp: internal error`. These indicate an internal inconsistency and are **not** part of the documented user-facing contract. Please report them as bugs.

## Related issues

This document addresses long-standing requests for clearer error-handling guidance ([#594](https://github.com/jbeder/yaml-cpp/issues/594), [#1122](https://github.com/jbeder/yaml-cpp/issues/1122), [#1395](https://github.com/jbeder/yaml-cpp/issues/1395)).

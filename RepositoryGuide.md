`yaml-cpp` naturally splits into a "core" library with two "frontend" APIs, and so that's how the code is organized.

## Core ##

```
hg clone https://code.google.com/p/yaml-cpp.core/
```

This provides parsing to the intermediate (and undocumented!) event API, as well as emitting.

## New API ##

```
hg clone https://code.google.com/p/yaml-cpp.new-api/
```

This is an upstream of the `core` repository. It provides the new `YAML::Node` API. See [Tutorial](Tutorial.md) for details on this API.

## Old API ##

```
hg clone https://code.google.com/p/yaml-cpp.old-api/
```

This is an upstream of the `core` repository. It provides the old `YAML::Node` API. See [HowToParseADocument](HowToParseADocument.md) for details on this API.

## Default ##


```
hg clone https://code.google.com/p/yaml-cpp/
```

This is a mirror of the `new-api` repository.
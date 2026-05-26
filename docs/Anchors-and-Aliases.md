# Anchors and aliases

YAML anchors (`&name`) and aliases (`*name`) tie multiple document nodes to the same data. yaml-cpp handles them in two different ways depending on which API you use.

## High-level `YAML::Node` API (typical application code)

`YAML::Load` builds a graph of `YAML::Node` values. Assigning one node to another can create an **alias**: both names refer to the **same** underlying node, so updates through either path are visible on both sides.

```cpp
YAML::Node doc = YAML::Load(R"(
seq: [one, two]
)");
doc["mirror"] = doc["seq"][0];
doc["seq"][0] = "updated";
// doc["mirror"] is also "updated"
```

This is the usual way to work with duplicated structure in config files. The node API does **not** expose the original YAML anchor **name** (for example `&fred`) as a string on `YAML::Node`; it only preserves shared identity.

See also the [Tutorial](Tutorial.md) section on building nodes with mirrors and self-aliases.

## Low-level events: anchor names and numeric ids

If you need the anchor **label** from the file (`&anchor`) or to distinguish alias events (`*anchor`) from the anchored node, parse with `YAML::Parser` and a custom `YAML::EventHandler`:

- `OnAnchor(mark, anchor_name)` — YAML anchor definition with its textual name.
- `OnAlias(mark, anchor_t id)` — alias reference; `id` matches the anchored node's `anchor_t`.
- `OnScalar` / `OnSequenceStart` / `OnMapStart` — also receive an `anchor_t` when the node is anchored (non-zero id).

`anchor_t` is a numeric id (`std::size_t`); use it to pair `OnAnchor` / anchored content with later `OnAlias` callbacks. `NullAnchor` (0) means no anchor on that event.

Minimal sketch:

```cpp
#include "yaml-cpp/eventhandler.h"
#include "yaml-cpp/parser.h"

#include <fstream>
#include <iostream>

class AnchorPrinter : public YAML::EventHandler {
 public:
  void OnDocumentStart(const YAML::Mark&) override {}
  void OnDocumentEnd() override {}

  void OnAnchor(const YAML::Mark&, const std::string& anchor_name) override {
    std::cout << "anchor defined: &" << anchor_name << "\n";
  }

  void OnAlias(const YAML::Mark&, YAML::anchor_t id) override {
    std::cout << "alias: * (id " << id << ")\n";
  }

  void OnNull(const YAML::Mark&, YAML::anchor_t) override {}
  void OnScalar(const YAML::Mark&, const std::string&, YAML::anchor_t,
                const std::string& value) override {
    std::cout << "scalar: " << value << "\n";
  }
  void OnSequenceStart(const YAML::Mark&, const std::string&, YAML::anchor_t,
                       YAML::EmitterStyle::value) override {}
  void OnSequenceEnd() override {}
  void OnMapStart(const YAML::Mark&, const std::string&, YAML::anchor_t,
                  YAML::EmitterStyle::value) override {}
  void OnMapEnd() override {}
};

int main() {
  std::ifstream in("example.yaml");
  YAML::Parser parser(in);
  AnchorPrinter handler;
  parser.HandleNextDocument(handler);
}
```

For a working parser driver, see `util/read.cpp` in this repository.

## Emitting anchors and aliases

To **write** anchors and aliases, use the emitter manipulators `YAML::Anchor` and `YAML::Alias` — see [How to Emit YAML](How-To-Emit-YAML.md) (section *And so do aliases/anchors*).

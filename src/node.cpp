#include "yaml-cpp/node/node.h"
#include "nodebuilder.h"
#include "nodeevents.h"

namespace YAML {
Node Clone(const Node& node, bool preserveMarks) {
  NodeEvents events(node);
  NodeBuilder builder;
  events.Emit(builder, preserveMarks);
  return builder.Root();
}
}  // namespace YAML

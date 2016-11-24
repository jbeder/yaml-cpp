#include "yaml-cpp/node/detail/memory.h"
#include "yaml-cpp/node/detail/node.h"  // IWYU pragma: keep
#include "yaml-cpp/node/ptr.h"

namespace YAML {
namespace detail {

typedef ref_holder<node> node_ref;

node& memory::create_node() {
  m_nodes.emplace_back();
  return m_nodes.back();
}

void memory::merge(memory& rhs) {
    m_nodes.splice(m_nodes.end(), rhs.m_nodes);
}

memory::memory() {}
memory::~memory() {}
}
}

#include "yaml-cpp/node/detail/memory.h"
#include "yaml-cpp/node/detail/node.h"  // IWYU pragma: keep
#include "yaml-cpp/node/ptr.h"

namespace YAML {
namespace detail {

typedef ref_holder<node> node_ref;

void memory_holder::merge(memory_holder& rhs) {
  if (m_pMemory == rhs.m_pMemory)
    return;

  m_pMemory->merge(std::move(*rhs.m_pMemory));
  rhs.m_pMemory = m_pMemory;
}

node& memory::create_node() {
  m_nodes.emplace_back();
  return m_nodes.back();
}

void memory::merge(memory&& rhs) { m_nodes.splice(m_nodes.end(), rhs.m_nodes); }

memory_holder::memory_holder() : m_pMemory(new memory) {}
memory_holder::~memory_holder() {}

memory::memory() {}
memory::~memory() {}
}
}

#include "yaml-cpp/node/detail/memory.h"
#include "yaml-cpp/node/detail/node.h"  // IWYU pragma: keep
#include "yaml-cpp/node/ptr.h"

namespace YAML {
namespace detail {

void memory_holder::merge(memory_holder& rhs) {
  if (m_pMemory == rhs.m_pMemory)
    return;

  if (m_pMemory->size() < rhs.m_pMemory->size()) {
    std::swap(m_pMemory, rhs.m_pMemory);
  }

  m_pMemory->merge(*rhs.m_pMemory);
  rhs.m_pMemory = m_pMemory;
}

node& memory::create_node() {
  shared_node pNode(new node);
  m_nodes.insert(pNode);
  return *pNode;
}

void memory::merge(const memory& rhs) {
  m_nodes.insert(rhs.m_nodes.begin(), rhs.m_nodes.end());
}

size_t memory::size() const {
    return m_nodes.size();
}
}  // namespace detail
}  // namespace YAML

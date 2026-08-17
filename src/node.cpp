
#include "yaml-cpp/node/node.h"
#include "nodebuilder.h"
#include "nodeevents.h"
#include "yaml-cpp/node/convert.h"
#include "yaml-cpp/node/detail/impl.h"
#include "yaml-cpp/node/detail/memory.h"
#include "yaml-cpp/node/detail/node.h"
#include "yaml-cpp/node/impl.h"
#include <cassert>
#include <unordered_set>

namespace YAML {
Node Clone(const Node &node) {
  NodeEvents events(node);
  NodeBuilder builder;
  events.Emit(builder);
  return builder.Root();
}

Node::Node(const Node &rhs, ShallowClone)
    : m_isValid(rhs.m_isValid),
      m_invalidKey(),
      m_pMemory(rhs.m_pMemory),
      m_pNode(&m_pMemory->create_node()) {
  if (!m_isValid)
    return;
  m_pNode->copy_contents(*rhs.m_pNode);
}

Node::Node(detail::node &rhs, detail::shared_memory_holder const &memory,
           ShallowClone)
    : m_isValid(true),
      m_invalidKey(),
      m_pMemory(memory),
      m_pNode(&m_pMemory->create_node()) {
  m_pNode->copy_contents(rhs);
}

Node::Node(const Node &rhs, DeepClone)
    : m_isValid(rhs.m_isValid),
      m_invalidKey(),
      m_pMemory(rhs.m_pMemory),
      m_pNode(&m_pMemory->create_node()) {
  if (!m_isValid)
    return;
  NodeType::value t = rhs.Type();
  if (t == NodeType::Sequence) {
    m_pNode->set_type(NodeType::Sequence);
    for (auto const &sub : rhs)
      m_pNode->seq_push_back(*Node(sub, DeepClone()).m_pNode);
  } else if (t == NodeType::Map) {
    m_pNode->set_type(NodeType::Map);
    for (auto const &kv : rhs)
      m_pNode->map_force_insert(
          *Node(kv.first, ShallowClone()).m_pNode,
          *Node(kv.second, DeepClone()).m_pNode);  // held in same memory
  } else
    m_pNode->copy_contents(*rhs.m_pNode);
}

Node::Node(detail::node &rhs, detail::shared_memory_holder const &memory,
           DeepClone)
    : m_isValid(true),
      m_invalidKey(),
      m_pMemory(memory),
      m_pNode(&m_pMemory->create_node()) {
  NodeType::value t = rhs.type();
  if (t == NodeType::Sequence) {
    m_pNode->set_type(NodeType::Sequence);
    for (auto const &sub : rhs)
      m_pNode->seq_push_back(
          *Node(*sub, m_pMemory, DeepClone()).m_pNode);  // held in same memory
  } else if (t == NodeType::Map) {
    m_pNode->set_type(NodeType::Map);
    for (auto const &kv : rhs)
      m_pNode->map_force_insert(
          *Node(*kv.first, m_pMemory, ShallowClone()).m_pNode,
          *Node(*kv.second, m_pMemory, DeepClone()).m_pNode);
  } else
    m_pNode->copy_contents(rhs);
}

Node::Node(const Node &rhs, YAML::UnshareSubtrees)
    : m_isValid(rhs.m_isValid),
      m_invalidKey(),
      m_pMemory(rhs.m_pMemory),
      m_pNode(&m_pMemory->create_node()) {
  if (!m_isValid)
    return;
  m_pNode->copy_contents(*rhs.m_pNode);
  this->UnshareSubtrees();
}

struct HashNodeId {
  std::size_t operator()(void *nodeId) const noexcept {
    return (std::size_t)nodeId >> 5;
  }
};

typedef std::unordered_set<void *, HashNodeId> SubtreeSeen;

#define YAML_CPP_UNSHARE_BY_MODIFY 1

void Node::OwnData() const {
  if (!m_isValid)
    return;
  auto p = m_pNode;
  (m_pNode = &m_pMemory->create_node())->copy_contents(*p);
}

#if YAML_CPP_UNSHARE_BY_MODIFY
static void unshare(detail::node &n, SubtreeSeen &seen,
                    detail::shared_memory_holder const &memory) {
  n.modify([&seen, &memory](detail::node *s) {
    if (seen.insert(s->id()).second) {
      unshare(*s, seen, memory);
      return s;
    } else
      return &Node(*s, memory, DeepClone()).node();
  });
}
#else
static void unshare(Node &n, SubtreeSeen &seen) {
  Node copied;
  bool modified = false;
  if (n.IsSequence()) {
    for (Node sub : n)
      if (!seen.insert(sub.id()).second) {
        modified = true;
        copied.push_back(Node(sub, DeepClone()));
      } else {
        unshare(sub, seen);
        copied.push_back(sub);
      }
  } else if (n.IsMap())
    for (auto const &kv : n) {
      Node sub = kv.second;
      if (!seen.insert(sub.id()).second) {
        modified = true;
        copied[kv.first] = Node(sub, DeepClone());
      } else {
        unshare(sub, seen);
        copied[kv.first] = sub;
      }
    }
  // TODO: ModifyValues or other faster impl
  if (modified)
    n = copied;
}
#endif

void Node::UnshareSubtrees() {
  SubtreeSeen seen;
#if YAML_CPP_UNSHARE_BY_MODIFY
  unshare(*m_pNode, seen, m_pMemory);
#else
  unshare(*this, seen);
#endif
}

void Node::ModifyValues(modify_values const &f) {
  if (!m_isValid)
    return;
  m_pNode->modify(f);
}

void Node::ModifyKeyValues(modify_key_values const &f) {
  if (!m_isValid)
    return;
  m_pNode->map_modify(f);
}

}  // namespace YAML

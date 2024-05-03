#include <algorithm>
#include <cassert>

#include "nodebuilder.h"
#include "yaml-cpp/node/detail/node.h"
#include "yaml-cpp/node/impl.h"
#include "yaml-cpp/node/node.h"
#include "yaml-cpp/node/type.h"

namespace YAML {
struct Mark;

NodeBuilder::NodeBuilder()
    : m_pMemory(new detail::memory_holder),
      m_pRoot(nullptr),
      m_stack{},
      m_anchors{},
      m_keys{},
      m_mergeDicts{},
      m_mapDepth(0) {
  m_anchors.push_back(nullptr);  // since the anchors start at 1
}

NodeBuilder::~NodeBuilder() = default;

Node NodeBuilder::Root() {
  if (!m_pRoot)
    return Node();

  return Node(*m_pRoot, m_pMemory);
}

void NodeBuilder::OnDocumentStart(const Mark&) {}

void NodeBuilder::OnDocumentEnd() {}

void NodeBuilder::OnNull(const Mark& mark, anchor_t anchor) {
  detail::node& node = Push(mark, anchor);
  node.set_null();
  Pop();
}

void NodeBuilder::OnAlias(const Mark& /* mark */, anchor_t anchor) {
  detail::node& node = *m_anchors[anchor];
  Push(node);
  Pop();
}

void NodeBuilder::OnScalar(const Mark& mark, const std::string& tag,
                           anchor_t anchor, const std::string& value) {
  detail::node& node = Push(mark, anchor);
  node.set_scalar(value);
  node.set_tag(tag);
  Pop();
}

void NodeBuilder::OnSequenceStart(const Mark& mark, const std::string& tag,
                                  anchor_t anchor, EmitterStyle::value style) {
  detail::node& node = Push(mark, anchor);
  node.set_tag(tag);
  node.set_type(NodeType::Sequence);
  node.set_style(style);
}

void NodeBuilder::OnSequenceEnd() { Pop(); }

void NodeBuilder::OnMapStart(const Mark& mark, const std::string& tag,
                             anchor_t anchor, EmitterStyle::value style) {
  detail::node& node = Push(mark, anchor);
  node.set_type(NodeType::Map);
  node.set_tag(tag);
  node.set_style(style);
  m_mapDepth++;
  m_mergeDicts.emplace_back();
}

void MergeMapCollection(detail::node& map_to, detail::node& map_from,
                        detail::shared_memory_holder& pMemory) {
  for (auto j = map_from.begin(); j != map_from.end(); j++) {
    const auto from_key = j->first;
    /// NOTE: const_map_to.get(*j->first) cannot be used here, since it
    /// compares only the shared_ptr's, while we need to compare the key
    /// itself.
    ///
    /// NOTE: get() also iterates over elements
    bool found = std::any_of(map_to.begin(), map_to.end(), [&](const detail::node_iterator_value<detail::node> & kv)
    {
        const auto key_node = kv.first;
        return key_node->scalar() == from_key->scalar();
    });
    if (!found)
      map_to.insert(*from_key, *j->second, pMemory);
  }
}

void NodeBuilder::OnMapEnd() {
  assert(m_mapDepth > 0);
  detail::node& collection = *m_stack.back();
  auto& toMerge = *m_mergeDicts.rbegin();
  for (detail::node* n : toMerge) {
    MergeMapCollection(collection, *n, m_pMemory);
  }
  m_mapDepth--;
  m_mergeDicts.pop_back();
  Pop();
}

detail::node& NodeBuilder::Push(const Mark& mark, anchor_t anchor) {
  detail::node& node = m_pMemory->create_node();
  node.set_mark(mark);
  RegisterAnchor(anchor, node);
  Push(node);
  return node;
}

void NodeBuilder::Push(detail::node& node) {
  const bool needsKey =
      (!m_stack.empty() && m_stack.back()->type() == NodeType::Map &&
       m_keys.size() < m_mapDepth);

  m_stack.push_back(&node);
  if (needsKey)
    m_keys.emplace_back(&node, false);
}

void NodeBuilder::Pop() {
  assert(!m_stack.empty());
  if (m_stack.size() == 1) {
    m_pRoot = m_stack[0];
    m_stack.pop_back();
    return;
  }

  detail::node& node = *m_stack.back();
  m_stack.pop_back();

  detail::node& collection = *m_stack.back();
  if (collection.type() == NodeType::Sequence) {
    collection.push_back(node, m_pMemory);
  } else if (collection.type() == NodeType::Map) {
    assert(!m_keys.empty());
    PushedKey& key = m_keys.back();
    if (key.second) {
      detail::node& nk = *key.first;
      if (nk.type() == NodeType::Scalar &&
          ((nk.tag() == "tag:yaml.org,2002:merge" && nk.scalar() == "<<") ||
           (nk.tag() == "?" && nk.scalar() == "<<"))) {
        if (node.type() == NodeType::Map) {
          m_mergeDicts.rbegin()->emplace_back(&node);
          m_keys.pop_back();
        } else if (node.type() == NodeType::Sequence) {
          for (auto i = node.begin(); i != node.end(); i++) {
            auto v = *i;
            if ((*v).type() == NodeType::Map) {
              m_mergeDicts.rbegin()->emplace_back(&(*v));
            } else {
              throw ParserException(
                  node.mark(),
                  ErrorMsg::MERGE_KEY_NEEDS_SINGLE_OR_SEQUENCE_OF_MAPS);
            }
          }
          m_keys.pop_back();
        } else {
          throw ParserException(
              node.mark(),
              ErrorMsg::MERGE_KEY_NEEDS_SINGLE_OR_SEQUENCE_OF_MAPS);
        }
      } else {
        collection.insert(*key.first, node, m_pMemory);
        m_keys.pop_back();
      }
    } else {
      key.second = true;
    }
  } else {
    assert(false);
    m_stack.clear();
  }
}

void NodeBuilder::RegisterAnchor(anchor_t anchor, detail::node& node) {
  if (anchor) {
    assert(anchor == m_anchors.size());
    m_anchors.push_back(&node);
  }
}
}  // namespace YAML

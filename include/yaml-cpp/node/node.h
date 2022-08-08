#ifndef NODE_NODE_H_62B23520_7C8E_11DE_8A39_0800200C9A66
#define NODE_NODE_H_62B23520_7C8E_11DE_8A39_0800200C9A66

#if defined(_MSC_VER) ||                                            \
    (defined(__GNUC__) && (__GNUC__ == 3 && __GNUC_MINOR__ >= 4) || \
     (__GNUC__ >= 4))  // GCC supports "pragma once" correctly since 3.4
#pragma once
#endif

#include <stdexcept>
#include <string>

#include "yaml-cpp/dll.h"
#include "yaml-cpp/emitterstyle.h"
#include "yaml-cpp/mark.h"
#include "yaml-cpp/node/detail/iterator_fwd.h"
#include "yaml-cpp/node/modify.h"
#include "yaml-cpp/node/ptr.h"
#include "yaml-cpp/node/type.h"
#include "yaml-cpp/noexcept.h"

namespace YAML {
namespace detail {
class node;
class node_data;
struct iterator_value;
}  // namespace detail
}  // namespace YAML

namespace YAML {
// Node ctor tags
struct UnshareSubtrees {};
struct ShallowClone {};
struct DeepClone {};

class YAML_CPP_API Node {
 public:
  friend class NodeBuilder;
  friend class NodeEvents;
  friend struct detail::iterator_value;
  friend class detail::node;
  friend class detail::node_data;
  template <typename>
  friend class detail::iterator_base;
  template <typename T, typename S>
  friend struct as_if;

  using iterator = YAML::iterator;
  using const_iterator = YAML::const_iterator;

  Node();
  explicit Node(NodeType::value type);
  template <typename T>
  explicit Node(const T& rhs);
  explicit Node(const detail::iterator_value& rhs);
  Node(const Node& rhs);
  // newly allocated shallow clone (copies data incl anchors/tags). subtrees may
  // still have sharing
  Node(const Node& rhs, ShallowClone);
  // Node(Node(rhs, memory), ShallowClone())
  Node(detail::node& rhs, detail::shared_memory_holder const& memory,
       ShallowClone);
  // newly allocated deep (recursive) clone (recurisvely copies data but
  // possibly not anchors/tags). no cycle detection (i.e. pathological input
  // with itself as subtree will expand infinitely)
  Node(const Node& rhs, DeepClone);
  // Node(Node(rhs, memory), DeepClone())
  Node(detail::node& rhs, detail::shared_memory_holder const& memory,
       DeepClone);
  // rhs but with no shared subtrees (may be is(rhs) if none were shared)
  Node(const Node& rhs, UnshareSubtrees);
  Node(detail::node& rhs, detail::shared_memory_holder const& memory);
  ~Node();

  detail::node& node() const { return *m_pNode; }
  detail::shared_memory_holder const& memory() const { return m_pMemory; }

  // \post no anchor-shared subtrees (all but one will be clones instead).
  void UnshareSubtrees();
  // \post *this = Node(*this, ShallowClone) - copy data.
  void OwnData() const;
  // f takes and returns a detail::node * for each value in sequence or map;
  // value is updated. does nothing for other types
  void ModifyValues(modify_values const& f);
  // \pre IsMap(). f(std::string const* key, detail::node* val)->detail::node*
  // as above but with key (if Scalar) else 0 arg
  void ModifyKeyValues(modify_key_values const& f);

  YAML::Mark Mark() const;
  NodeType::value Type() const;
  bool IsDefined() const;
  bool IsNull() const { return Type() == NodeType::Null; }
  bool IsScalar() const { return Type() == NodeType::Scalar; }
  bool IsSequence() const { return Type() == NodeType::Sequence; }
  bool IsMap() const { return Type() == NodeType::Map; }

  // bool conversions
  explicit operator bool() const { return IsDefined(); }
  bool operator!() const { return !IsDefined(); }

  // access
  template <typename T>
  T as() const;
  template <typename T, typename S>
  T as(const S& fallback) const;
  const std::string& Scalar() const;

  const std::string& Tag() const;
  void SetTag(const std::string& tag);

  // style
  // WARNING: This API might change in future releases.
  EmitterStyle::value Style() const;
  void SetStyle(EmitterStyle::value style);

  // a.id() == b.id() <=> a.is(b). \pre valid
  void* id() const YAML_CPP_NOEXCEPT;
  friend inline std::size_t hash_value(Node const& n) YAML_CPP_NOEXCEPT {
    return n.hash();
  }
  std::size_t hash() const YAML_CPP_NOEXCEPT { return (std::size_t)id() >> 5; }

  // assignment
  // note: default/invalid nodes return false; undefined may return true
  bool is(const Node& rhs) const;
  template <typename T>
  Node& operator=(const T& rhs);
  Node& operator=(const Node& rhs);
  void reset(const Node& rhs = Node());

  // size/iterator
  std::size_t size() const;

  const_iterator begin() const;
  iterator begin();

  const_iterator end() const;
  iterator end();

  // sequence
  template <typename T>
  void push_back(const T& rhs);
  void push_back(const Node& rhs);
  // \pre IsSequence
  void seq_push_back(const Node& rhs);

  // indexing
  template <typename Key>
  const Node operator[](const Key& key) const;
  template <typename Key>
  Node operator[](const Key& key);
  template <typename Key>
  bool remove(const Key& key);

  const Node operator[](const Node& key) const;
  Node operator[](const Node& key);
  bool remove(const Node& key);

  // map
  template <typename Key, typename Value>
  void force_insert(const Key& key, const Value& value);
  // \pre IsMap()
  void map_force_insert(Node const& key, Node const& value);

 private:
  enum Zombie { ZombieNode };
  explicit Node(Zombie);
  explicit Node(Zombie, const std::string&);

  void EnsureNodeExists() const;

  template <typename T>
  void Assign(const T& rhs);
  void Assign(const char* rhs);
  void Assign(char* rhs);

  void AssignData(const Node& rhs);
  void AssignNode(const Node& rhs);

 private:
  bool m_isValid;
  // String representation of invalid key, if the node is invalid.
  std::string m_invalidKey;
  mutable detail::shared_memory_holder m_pMemory;
  mutable detail::node* m_pNode;
};

// lhs.is(rhs)
YAML_CPP_API bool operator==(const Node& lhs, const Node& rhs);

// keeps anchors etc intact
YAML_CPP_API Node Clone(const Node& node);

template <typename T>
struct convert;
}  // namespace YAML

namespace std {
template <>
struct hash<YAML::Node> {
  std::size_t operator()(YAML::Node const& n) { return n.hash(); }
  std::size_t operator()(YAML::Node const* n) { return n->hash(); }
};
}  // namespace std

#endif  // NODE_NODE_H_62B23520_7C8E_11DE_8A39_0800200C9A66

#ifndef NODE_DETAIL_NODE_H_62B23520_7C8E_11DE_8A39_0800200C9A66
#define NODE_DETAIL_NODE_H_62B23520_7C8E_11DE_8A39_0800200C9A66

#if defined(_MSC_VER) ||                                            \
    (defined(__GNUC__) && (__GNUC__ == 3 && __GNUC_MINOR__ >= 4) || \
     (__GNUC__ >= 4))  // GCC supports "pragma once" correctly since 3.4
#pragma once
#endif

#include "yaml-cpp/emitterstyle.h"
#include "yaml-cpp/dll.h"
#include "yaml-cpp/node/type.h"
#include "yaml-cpp/node/ptr.h"
#include "yaml-cpp/node/detail/node_ref.h"
#include <set>

namespace YAML {
namespace detail {
class node : public node_ref_id {
 public:
  node() : node_ref_id(std::make_shared<node_ref>()) {}

  bool is(const node& rhs) const { return m_pRef == rhs.m_pRef; }

  bool is_defined() const { return nref().is_defined(); }
  const Mark& mark() const { return nref().mark(); }
  NodeType::value type() const { return nref().type(); }

  const std::string& scalar() const { return nref().scalar(); }
  const std::string& tag() const { return nref().tag(); }
  EmitterStyle::value style() const { return nref().style(); }

  template <typename T>
  bool equals(const T& rhs, shared_memory_holder pMemory);
  bool equals(const char* rhs, shared_memory_holder pMemory);

  void mark_defined() {
    if (is_defined())
      return;

    nref().mark_defined();
    for (nodes::iterator it = m_dependencies.begin();
         it != m_dependencies.end(); ++it)
      (*it)->mark_defined();
    m_dependencies.clear();
  }

  void add_dependency(node& rhs) {
    if (is_defined())
      rhs.mark_defined();
    else
      m_dependencies.insert(&rhs);
  }

  void set_ref(const node& rhs) {
    if (rhs.is_defined())
      mark_defined();
    m_pRef = rhs.m_pRef;
  }
  void set_data(const node& rhs) {
    if (rhs.is_defined())
      mark_defined();
    nref().set_data(rhs.nref());
  }

  void set_mark(const Mark& mark) { nref().set_mark(mark); }

  void set_type(NodeType::value type) {
    if (type != NodeType::Undefined)
      mark_defined();
    nref().set_type(type);
  }
  void set_null() {
    mark_defined();
    nref().set_null();
  }
  void set_scalar(const std::string& scalar) {
    mark_defined();
    nref().set_scalar(scalar);
  }
  void set_tag(const std::string& tag) {
    mark_defined();
    nref().set_tag(tag);
  }

  // style
  void set_style(EmitterStyle::value style) {
    mark_defined();
    nref().set_style(style);
  }

  // size/iterator
  std::size_t size() const { return nref().size(); }

  const_node_iterator begin() const {
    return static_cast<const node_ref&>(nref()).begin();
  }
  node_iterator begin() { return nref().begin(); }

  const_node_iterator end() const {
    return static_cast<const node_ref&>(nref()).end();
  }
  node_iterator end() { return nref().end(); }

  // sequence
  void push_back(node& input, shared_memory_holder pMemory) {
    nref().push_back(input, pMemory);
    input.add_dependency(*this);
  }
  void insert(node& key, node& value, shared_memory_holder pMemory) {
    nref().insert(key, value, pMemory);
    key.add_dependency(*this);
    value.add_dependency(*this);
  }

  // indexing
  template <typename Key>
  node* get(const Key& key, shared_memory_holder pMemory) const {
    // NOTE: this returns a non-const node so that the top-level Node can wrap
    // it, and returns a pointer so that it can be NULL (if there is no such
    // key).
    return static_cast<const node_ref&>(nref()).get(key, pMemory);
  }
  template <typename Key>
  node& get(const Key& key, shared_memory_holder pMemory) {
    node& value = nref().get(key, pMemory);
    value.add_dependency(*this);
    return value;
  }
  template <typename Key>
  bool remove(const Key& key, shared_memory_holder pMemory) {
    return nref().remove(key, pMemory);
  }

  node* get(node& key, shared_memory_holder pMemory) const {
    // NOTE: this returns a non-const node so that the top-level Node can wrap
    // it, and returns a pointer so that it can be NULL (if there is no such
    // key).
    return static_cast<const node_ref&>(nref()).get(key, pMemory);
  }
  node& get(node& key, shared_memory_holder pMemory) {
    node& value = nref().get(key, pMemory);
    key.add_dependency(*this);
    value.add_dependency(*this);
    return value;
  }
  bool remove(node& key, shared_memory_holder pMemory) {
    return nref().remove(key, pMemory);
  }

  // map
  template <typename Key, typename Value>
  void force_insert(const Key& key, const Value& value,
                    shared_memory_holder pMemory) {
    nref().force_insert(key, value, pMemory);
  }

 private:
  typedef std::set<node*> nodes;
  nodes m_dependencies;
};
}
}

#endif  // NODE_DETAIL_NODE_H_62B23520_7C8E_11DE_8A39_0800200C9A66

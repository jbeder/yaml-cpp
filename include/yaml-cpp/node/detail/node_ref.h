#ifndef VALUE_DETAIL_NODE_REF_H_62B23520_7C8E_11DE_8A39_0800200C9A66
#define VALUE_DETAIL_NODE_REF_H_62B23520_7C8E_11DE_8A39_0800200C9A66

#if defined(_MSC_VER) ||                                            \
    (defined(__GNUC__) && (__GNUC__ == 3 && __GNUC_MINOR__ >= 4) || \
     (__GNUC__ >= 4))  // GCC supports "pragma once" correctly since 3.4
#pragma once
#endif

#include "yaml-cpp/dll.h"
#include "yaml-cpp/node/type.h"
#include "yaml-cpp/node/ptr.h"
#include "yaml-cpp/node/detail/node_data.h"

namespace YAML {
namespace detail {
class node_ref {
 public:
  node_ref() : m_pData(new node_data) {}
  node_ref(const node_ref&) = delete;
  node_ref& operator=(const node_ref&) = delete;

  bool is_defined() const { return m_pData->is_defined(); }
  const Mark& mark() const { return m_pData->mark(); }
  NodeType::value type() const { return m_pData->type(); }
  const std::string& scalar() const { return m_pData->scalar(); }
  const std::string& tag() const { return m_pData->tag(); }
  EmitterStyle::value style() const { return m_pData->style(); }

  // needed if we allow copy_contents
  void *id() const YAML_CPP_NOEXCEPT { return m_pData.get(); }
  void mark_defined() { m_pData->mark_defined(); }
  void set_data(const node_ref& rhs) { m_pData = rhs.m_pData; }
  void copy_contents(const node_ref& rhs) {
    if (&rhs == this) return;
    node_data *r = rhs.m_pData.get();
    m_pData = std::make_shared<node_data>(*r);
  }

  void set_mark(const Mark& mark) { m_pData->set_mark(mark); }
  void set_type(NodeType::value type) { m_pData->set_type(type); }
  void set_tag(const std::string& tag) { m_pData->set_tag(tag); }
  void set_null() { m_pData->set_null(); }
  void set_scalar(const std::string& scalar) { m_pData->set_scalar(scalar); }
  void set_style(EmitterStyle::value style) { m_pData->set_style(style); }

  // size/iterator
  std::size_t size() const { return m_pData->size(); }

  const_node_iterator begin() const {
    return static_cast<const node_data&>(*m_pData).begin();
  }
  node_iterator begin() { return m_pData->begin(); }

  const_node_iterator end() const {
    return static_cast<const node_data&>(*m_pData).end();
  }
  node_iterator end() { return m_pData->end(); }

  void modify(modify_values const& f) {
    m_pData->modify(f);
  }

  void map_modify(modify_key_values const& f) {
    m_pData->map_modify(f);
  }

  // sequence
  void push_back(node& node, shared_memory_holder pMemory) {
    m_pData->push_back(node, pMemory);
  }
  // \pre IsSequence, node in our memory
  void seq_push_back(node& node) {
    m_pData->seq_push_back(node);
  }

  // map
  void insert(node& key, node& value, shared_memory_holder pMemory) {
    m_pData->insert(key, value, pMemory);
  }
  // \pre IsMap, key value held in our memory
  void map_force_insert(node& key, node& value) {
    m_pData->map_force_insert(key, value);
  }

  // indexing
  template <typename Key>
  node* get(const Key& key, shared_memory_holder pMemory) const {
    return static_cast<const node_data&>(*m_pData).get(key, pMemory);
  }
  template <typename Key>
  node& get(const Key& key, shared_memory_holder pMemory) {
    return m_pData->get(key, pMemory);
  }
  template <typename Key>
  bool remove(const Key& key, shared_memory_holder pMemory) {
    return m_pData->remove(key, pMemory);
  }

  node* get(node& key, shared_memory_holder pMemory) const {
    return static_cast<const node_data&>(*m_pData).get(key, pMemory);
  }
  node& get(node& key, shared_memory_holder pMemory) {
    return m_pData->get(key, pMemory);
  }
  bool remove(node& key, shared_memory_holder pMemory) {
    return m_pData->remove(key, pMemory);
  }

  // map
  template <typename Key, typename Value>
  void force_insert(const Key& key, const Value& value,
                    shared_memory_holder pMemory) {
    m_pData->force_insert(key, value, pMemory);
  }

 private:
  shared_node_data m_pData;
};
}
}

#endif  // VALUE_DETAIL_NODE_REF_H_62B23520_7C8E_11DE_8A39_0800200C9A66

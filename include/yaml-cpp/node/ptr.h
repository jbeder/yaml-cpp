#ifndef VALUE_PTR_H_62B23520_7C8E_11DE_8A39_0800200C9A66
#define VALUE_PTR_H_62B23520_7C8E_11DE_8A39_0800200C9A66

#if defined(_MSC_VER) ||                                            \
    (defined(__GNUC__) && (__GNUC__ == 3 && __GNUC_MINOR__ >= 4) || \
     (__GNUC__ >= 4))  // GCC supports "pragma once" correctly since 3.4
#pragma once
#endif

#include "yaml-cpp/dll.h"
#include <memory>

namespace YAML {
namespace detail {
class node;
class node_ref;
class node_data;
class memory;
class memory_holder;

class node_ref_id {
protected:
  typedef std::shared_ptr<void> Ptr;
  Ptr m_pRef;
  node_ref_id(Ptr && p) : m_pRef((Ptr &&)p) {}
  node_ref_id(const node_ref_id&) = delete;
  node_ref_id& operator=(const node_ref_id&) = delete;
  detail::node_ref& nref() const { return *ref(); }
public:
  detail::node_ref* ref() const { return (detail::node_ref*)m_pRef.get(); }
  const void* id() const {
    return m_pRef.get();
  }
};

inline node_ref_id const* ref_id(node const* node) {
  // this could be static_cast if we pulled in the full definition of node
  return reinterpret_cast<node_ref_id const*>(node);
}

typedef std::shared_ptr<node> shared_node;
typedef std::shared_ptr<node_ref> shared_node_ref;
typedef std::shared_ptr<node_data> shared_node_data;
typedef std::shared_ptr<memory_holder> shared_memory_holder;
typedef std::shared_ptr<memory> shared_memory;
}
}

#endif  // VALUE_PTR_H_62B23520_7C8E_11DE_8A39_0800200C9A66

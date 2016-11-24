#ifndef VALUE_DETAIL_MEMORY_H_62B23520_7C8E_11DE_8A39_0800200C9A66
#define VALUE_DETAIL_MEMORY_H_62B23520_7C8E_11DE_8A39_0800200C9A66

#if defined(_MSC_VER) ||                                            \
    (defined(__GNUC__) && (__GNUC__ == 3 && __GNUC_MINOR__ >= 4) || \
     (__GNUC__ >= 4))  // GCC supports "pragma once" correctly since 3.4
#pragma once
#endif

#include <list>

#include "yaml-cpp/dll.h"
#include "yaml-cpp/node/ptr.h"

namespace YAML {
namespace detail {
class node;

}  // namespace detail
}  // namespace YAML

namespace YAML {
namespace detail {

class YAML_CPP_API memory : public ref_counted {
 public:
  node& create_node();
  void merge(memory&& rhs);

  memory();
  ~memory();

 private:
  typedef std::list<node> Nodes;
  Nodes m_nodes;
};

typedef ref_holder<memory> shared_memory;

class YAML_CPP_API memory_holder : public ref_counted {
 public:
  memory_holder();
  ~memory_holder();

  node& create_node() { return m_pMemory->create_node(); }
  void merge(memory_holder& rhs);

 private:
  shared_memory m_pMemory;
};

typedef ref_holder<memory_holder> shared_memory_holder;
}
}

#endif  // VALUE_DETAIL_MEMORY_H_62B23520_7C8E_11DE_8A39_0800200C9A66

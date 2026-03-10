#ifndef VALUE_PTR_H_62B23520_7C8E_11DE_8A39_0800200C9A66
#define VALUE_PTR_H_62B23520_7C8E_11DE_8A39_0800200C9A66

#if defined(_MSC_VER) ||                                            \
    (defined(__GNUC__) && (__GNUC__ == 3 && __GNUC_MINOR__ >= 4) || \
     (__GNUC__ >= 4))  // GCC supports "pragma once" correctly since 3.4
#pragma once
#endif

#include <memory>

namespace YAML {
namespace detail {
class node;
class node_ref;
class node_data;
class memory;
class memory_holder;

using shared_node = std::shared_ptr<node>;
using shared_node_ref = std::shared_ptr<node_ref>;
using shared_node_data = std::shared_ptr<node_data>;
using shared_memory_holder = std::shared_ptr<memory_holder>;
using shared_memory = std::shared_ptr<memory>;
}
}

#if defined(__has_cpp_attribute) && __has_cpp_attribute(msvc::lifetimebound)
#define YAML_ATTRIBUTE_LIFETIME_BOUND [[msvc::lifetimebound]]
#elif defined(__has_cpp_attribute) && __has_cpp_attribute(gnu::lifetimebound)
#define YAML_ATTRIBUTE_LIFETIME_BOUND [[gnu::lifetimebound]]
#elif defined(__has_cpp_attribute) && __has_cpp_attribute(clang::lifetimebound)
#define YAML_ATTRIBUTE_LIFETIME_BOUND [[clang::lifetimebound]]
#elif defined(__has_attribute) && __has_attribute(lifetimebound)
#define YAML_ATTRIBUTE_LIFETIME_BOUND __attribute__((lifetimebound))
#else
#define YAML_ATTRIBUTE_LIFETIME_BOUND
#endif

#endif  // VALUE_PTR_H_62B23520_7C8E_11DE_8A39_0800200C9A66

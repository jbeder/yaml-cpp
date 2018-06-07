#ifndef NODE_HASH_H_62B23520_7C8E_11DE_8A39_0800200C9A66
#define NODE_HASH_H_62B23520_7C8E_11DE_8A39_0800200C9A66

#include "yaml-cpp/node/detail/node.h"
#include "yaml-cpp/node/node.h"

namespace std {
template <>
struct hash<YAML::detail::node> {
  size_t operator()(const YAML::detail::node& key) const noexcept {
    return hash<YAML::detail::shared_node_ref>()(key.m_pRef);
  }
};

template <>
struct hash<YAML::Node> {
  size_t operator()(const YAML::Node& key) const noexcept {
    return hash<YAML::detail::node>()(*key.m_pNode);
  }
};
}  // namespace std

#endif  // NODE_HASH_H_62B23520_7C8E_11DE_8A39_0800200C9A66

#ifndef YAMLCPP__NODE_MODIFY_H
#define YAMLCPP__NODE_MODIFY_H

#if defined(_MSC_VER) ||                                            \
    (defined(__GNUC__) && (__GNUC__ == 3 && __GNUC_MINOR__ >= 4) || \
     (__GNUC__ >= 4))  // GCC supports "pragma once" correctly since 3.4
#pragma once
#endif

#include <functional>

namespace YAML {
namespace detail {
class node;
}  // namespace detail
}  // namespace YAML

namespace YAML {
/// \return 0 to remove key, identity to leave value unchanged, or new node to replace old value
typedef std::function<detail::node *(detail::node *)> modify_values;
/// as above but first arg is pointer to string key (or 0 if non-scalar)
typedef std::function<detail::node *(std::string const*, detail::node *)> modify_key_values;
}


#endif

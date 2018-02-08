#ifndef NODE_CONVERT_H_62B23520_7C8E_11DE_8A39_0800200C9A66
#define NODE_CONVERT_H_62B23520_7C8E_11DE_8A39_0800200C9A66

#if defined(_MSC_VER) ||                                            \
    (defined(__GNUC__) && (__GNUC__ == 3 && __GNUC_MINOR__ >= 4) || \
     (__GNUC__ >= 4))  // GCC supports "pragma once" correctly since 3.4
#pragma once
#endif

#include <array>
#include <bitset>
#include <cassert>
#include <deque>
#include <forward_list>
#include <limits>
#include <list>
#include <map>
#include <set>
#include <sstream>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "yaml-cpp/binary.h"
#include "yaml-cpp/node/impl.h"
#include "yaml-cpp/node/iterator.h"
#include "yaml-cpp/node/node.h"
#include "yaml-cpp/node/type.h"
#include "yaml-cpp/null.h"

namespace YAML {
class Binary;
struct _Null;
template <typename T>
struct convert;
}  // namespace YAML

namespace YAML {
namespace conversion {
inline bool IsInfinity(const std::string& input) {
  return input == ".inf" || input == ".Inf" || input == ".INF" ||
         input == "+.inf" || input == "+.Inf" || input == "+.INF";
}

inline bool IsNegativeInfinity(const std::string& input) {
  return input == "-.inf" || input == "-.Inf" || input == "-.INF";
}

inline bool IsNaN(const std::string& input) {
  return input == ".nan" || input == ".NaN" || input == ".NAN";
}
}

// Node
template <>
struct convert<Node> {
  static Node encode(const Node& rhs) { return rhs; }

  static bool decode(const Node& node, Node& rhs) {
    rhs.reset(node);
    return true;
  }
};

// std::string
template <>
struct convert<std::string> {
  static Node encode(const std::string& rhs) { return Node(rhs); }

  static bool decode(const Node& node, std::string& rhs) {
    if (!node.IsScalar())
      return false;
    rhs = node.Scalar();
    return true;
  }
};

// C-strings can only be encoded
template <>
struct convert<const char*> {
  static Node encode(const char*& rhs) { return Node(rhs); }
};

template <std::size_t N>
struct convert<const char[N]> {
  static Node encode(const char (&rhs)[N]) { return Node(rhs); }
};

template <>
struct convert<_Null> {
  static Node encode(const _Null& /* rhs */) { return Node(); }

  static bool decode(const Node& node, _Null& /* rhs */) {
    return node.IsNull();
  }
};

#define YAML_DEFINE_CONVERT_STREAMABLE(type, negative_op)                \
  template <>                                                            \
  struct convert<type> {                                                 \
    static Node encode(const type& rhs) {                                \
      std::stringstream stream;                                          \
      stream.precision(std::numeric_limits<type>::digits10 + 1);         \
      stream << rhs;                                                     \
      return Node(stream.str());                                         \
    }                                                                    \
                                                                         \
    static bool decode(const Node& node, type& rhs) {                    \
      if (node.Type() != NodeType::Scalar)                               \
        return false;                                                    \
      const std::string& input = node.Scalar();                          \
      std::stringstream stream(input);                                   \
      stream.unsetf(std::ios::dec);                                      \
      if ((stream >> std::noskipws >> rhs) && (stream >> std::ws).eof()) \
        return true;                                                     \
      if (std::numeric_limits<type>::has_infinity) {                     \
        if (conversion::IsInfinity(input)) {                             \
          rhs = std::numeric_limits<type>::infinity();                   \
          return true;                                                   \
        } else if (conversion::IsNegativeInfinity(input)) {              \
          rhs = negative_op std::numeric_limits<type>::infinity();       \
          return true;                                                   \
        }                                                                \
      }                                                                  \
                                                                         \
      if (std::numeric_limits<type>::has_quiet_NaN &&                    \
          conversion::IsNaN(input)) {                                    \
        rhs = std::numeric_limits<type>::quiet_NaN();                    \
        return true;                                                     \
      }                                                                  \
                                                                         \
      return false;                                                      \
    }                                                                    \
  }

#define YAML_DEFINE_CONVERT_STREAMABLE_SIGNED(type) \
  YAML_DEFINE_CONVERT_STREAMABLE(type, -)

#define YAML_DEFINE_CONVERT_STREAMABLE_UNSIGNED(type) \
  YAML_DEFINE_CONVERT_STREAMABLE(type, +)

YAML_DEFINE_CONVERT_STREAMABLE_SIGNED(int);
YAML_DEFINE_CONVERT_STREAMABLE_SIGNED(short);
YAML_DEFINE_CONVERT_STREAMABLE_SIGNED(long);
YAML_DEFINE_CONVERT_STREAMABLE_SIGNED(long long);
YAML_DEFINE_CONVERT_STREAMABLE_UNSIGNED(unsigned);
YAML_DEFINE_CONVERT_STREAMABLE_UNSIGNED(unsigned short);
YAML_DEFINE_CONVERT_STREAMABLE_UNSIGNED(unsigned long);
YAML_DEFINE_CONVERT_STREAMABLE_UNSIGNED(unsigned long long);

YAML_DEFINE_CONVERT_STREAMABLE_SIGNED(char);
YAML_DEFINE_CONVERT_STREAMABLE_SIGNED(signed char);
YAML_DEFINE_CONVERT_STREAMABLE_UNSIGNED(unsigned char);

YAML_DEFINE_CONVERT_STREAMABLE_SIGNED(float);
YAML_DEFINE_CONVERT_STREAMABLE_SIGNED(double);
YAML_DEFINE_CONVERT_STREAMABLE_SIGNED(long double);

#undef YAML_DEFINE_CONVERT_STREAMABLE_SIGNED
#undef YAML_DEFINE_CONVERT_STREAMABLE_UNSIGNED
#undef YAML_DEFINE_CONVERT_STREAMABLE

// bool
template <>
struct convert<bool> {
  static Node encode(bool rhs) { return rhs ? Node("true") : Node("false"); }

  YAML_CPP_API static bool decode(const Node& node, bool& rhs);
};

// Helper class for converting something which is "sequence-like" to and from a
// Node.
template <class T>
struct convert_as_sequence {
  static Node encode(const T& sequence) {
    Node node(NodeType::Sequence);
    for (const auto& item : sequence) {
      node.push_back(item);
    }
    return node;
  }
  static bool decode(const Node& node, T& sequence) {
    if (!node.IsSequence())
      return false;
    sequence.clear();
    for (const auto& item : node) {
      sequence.push_back(item.template as<typename T::value_type>());
    }
    return true;
  }
};

// Helper class for converting something which is "map-like" to and from a Node.
template <class T, class Key = typename T::key_type,
          class Value = typename T::mapped_type>
struct convert_as_map {
  static Node encode(const T& map) {
    Node node(NodeType::Map);
    for (const auto& kv : map) {
      node.force_insert(kv.first, kv.second);
    }
    return node;
  }

  static bool decode(const Node& node, T& map) {
    if (!node.IsMap())
      return false;
    map.clear();
    for (const auto& kv : node) {
// if we're dealing with GCC (note: clang also defines __GNUC__)
#if defined(__GNUC__)
#if __GNUC__ < 4
      // gcc version < 4
      map.insert(std::make_pair(kv.first.template as<Key>(),
                                kv.second.template as<Value>()));
#elif __GNUC__ == 4 && __GNUC_MINOR__ < 8
      // 4.0 <= gcc version < 4.8
      map.insert(std::make_pair(kv.first.as<Key>(), kv.second.as<Value>()));
#else
      // 4.8 <= gcc version
      map.emplace(kv.first.as<Key>(), kv.second.as<Value>());
#endif  // __GNUC__ < 4
#else
      // for anything not GCC or clang...
      // probably some more #ifdef guards are needed for MSVC.
      map.emplace(kv.first.as<Key>(), kv.second.as<Value>());
#endif  // defined(__GNUC__)
    }
    return true;
  }
};

// Helper class for converting something which is "set-like" to and from a Node.
// A set is realized as a yaml sequence.
template <class T>
struct convert_as_set {
  static Node encode(const T& set) {
    Node node(NodeType::Sequence);
    for (const auto& item : set) {
      node.push_back(item);
    }
    return node;
  }

  static bool decode(const Node& node, T& set) {
    if (!node.IsSequence())
      return false;

    set.clear();
    for (const auto& item : node) {

#if defined(__GNUC__) && __GNUC__ < 4
      // workaround for GCC 3:
      set.insert(item.template as<typename T::value_type>());
#else  // __GNUC__ is not defined or __GNUC__ >= 4
      set.insert(item.as<typename T::value_type>());
#endif
    }
    return true;
  }
};

// std::vector
template <typename T, class Alloc>
struct convert<std::vector<T, Alloc>>
    : public convert_as_sequence<std::vector<T, Alloc>> {};

// std::deque
template <class T, class Alloc>
struct convert<std::deque<T, Alloc>>
    : public convert_as_sequence<std::deque<T, Alloc>> {};

// std::list
template <typename T, class Alloc>
struct convert<std::list<T, Alloc>>
    : public convert_as_sequence<std::list<T, Alloc>> {};

// std::map
template <class Key, class T, class Compare, class Alloc>
struct convert<std::map<Key, T, Compare, Alloc>>
    : public convert_as_map<std::map<Key, T, Compare, Alloc>> {};

// std::unordered_map
template <class Key, class T, class Hash, class KeyEqual, class Alloc>
struct convert<std::unordered_map<Key, T, Hash, KeyEqual, Alloc>>
    : public convert_as_map<std::unordered_map<Key, T, Hash, KeyEqual, Alloc>> {
};

// std::set
template <class Key, class Compare, class Alloc>
struct convert<std::set<Key, Compare, Alloc>>
    : public convert_as_set<std::set<Key, Compare, Alloc>> {};

// std::unordered_set
template <class Key, class Hash, class KeyEqual, class Alloc>
struct convert<std::unordered_set<Key, Hash, KeyEqual, Alloc>>
    : public convert_as_set<std::unordered_set<Key, Hash, KeyEqual, Alloc>> {};

// std::forward_list
template <class T, class Alloc>
struct convert<std::forward_list<T, Alloc>> {
  static Node encode(const std::forward_list<T, Alloc>& sequence) {
    Node node(NodeType::Sequence);
    for (const auto& item : sequence) {
      node.push_back(item);
    }
    return node;
  }

  static bool decode(const Node& node, std::forward_list<T, Alloc>& sequence) {
    if (!node.IsSequence())
      return false;

    sequence.clear();

    // Walk the node backwards, because std::forward_list does not have
    // push_back, only push_front.
    for (std::size_t i = node.size() - 1; i != (std::size_t)-1; --i) {
#if defined(__GNUC__) && __GNUC__ < 4
      // workaround for GCC 3:
      sequence.push_front(node[i].template as<T>());
#else
      sequence.push_front(node[i].as<T>());
#endif
    }
    return true;
  }
};

// std::array
template <typename T, std::size_t N>
struct convert<std::array<T, N>> {
  static Node encode(const std::array<T, N>& sequence) {
    Node node(NodeType::Sequence);
    for (const auto& item : sequence) {
      node.push_back(item);
    }
    return node;
  }

  static bool decode(const Node& node, std::array<T, N>& sequence) {
    if (!isNodeValid(node)) {
      return false;
    }

    for (auto i = 0u; i < node.size(); ++i) {
#if defined(__GNUC__) && __GNUC__ < 4
      // workaround for GCC 3:
      sequence[i] = node[i].template as<T>();
#else
      sequence[i] = node[i].as<T>();
#endif
    }
    return true;
  }

 private:
  static bool isNodeValid(const Node& node) {
    return node.IsSequence() && node.size() == N;
  }
};

// std::bitset
template <std::size_t N>
struct convert<std::bitset<N>> {
  using value_type = std::bitset<N>;

  static Node encode(const value_type& rhs) {
    return convert<std::string>::encode(rhs.to_string());
  }

  static bool decode(const Node& node, value_type& rhs) {
    std::string representation;

    if (!convert<std::string>::decode(node, representation))
      return false;
    if (representation.size() != N)
      return false;
    try {
      // bitset constructor will throw std:invalid_argument if the decoded
      // string contains characters other than 0 and 1.
      rhs = value_type(representation);
    } catch (const std::invalid_argument& /*error*/) {
      return false;
    }
    return true;
  }
};

namespace detail {

template <std::size_t Index = 0, typename... Args>
inline typename std::enable_if<Index == sizeof...(Args), void>::type
    encode_tuple(Node& /*node*/, const std::tuple<Args...>& /*tup*/) {}

template <std::size_t Index = 0, typename... Args>
inline typename std::enable_if<Index != sizeof...(Args), void>::type
    encode_tuple(Node& node, const std::tuple<Args...>& tup) {
  node.push_back(std::get<Index>(tup));
  encode_tuple<Index + 1, Args...>(node, tup);
}

template <std::size_t Index = 0, typename... Args>
inline typename std::enable_if<Index == sizeof...(Args), void>::type
    decode_tuple(const Node& /*node*/, std::tuple<Args...>& /*tup*/) {}

template <std::size_t Index = 0, typename... Args>
inline typename std::enable_if<Index != sizeof...(Args), void>::type
    decode_tuple(const Node& node, std::tuple<Args...>& tup) {
  std::get<Index>(tup) =
      node[Index]
          .template as<
              typename std::tuple_element<Index, std::tuple<Args...>>::type>();
  decode_tuple<Index + 1, Args...>(node, tup);
}

}  // namespace detail

// std::tuple
template <typename... Args>
struct convert<std::tuple<Args...>> {
  static Node encode(const std::tuple<Args...>& tup) {
    static_assert(sizeof...(Args) > 0,
                  "wrong template specialization selected");
    Node node(NodeType::Sequence);
    detail::encode_tuple(node, tup);
    return node;
  }

  static bool decode(const Node& node, std::tuple<Args...>& tup) {
    static_assert(sizeof...(Args) > 0,
                  "wrong template specialization selected");
    if (!node.IsSequence() || node.size() != sizeof...(Args))
      return false;
    detail::decode_tuple(node, tup);
    return true;
  }
};

// std::tuple -- empty
template <>
struct convert<std::tuple<>> {
  static Node encode(const std::tuple<>& /*tup*/) {
    return convert<_Null>::encode(Null);
  }
  static bool decode(const Node& node, std::tuple<>& /*tup*/) {
    return convert<_Null>::decode(node, Null);
  }
};

// std::pair -- special case of std::tuple
template <class First, class Second>
struct convert<std::pair<First, Second>> {
  static Node encode(const std::pair<First, Second>& tup) {
    Node node(NodeType::Sequence);
    node.push_back(std::get<0>(tup));
    node.push_back(std::get<1>(tup));
    return node;
  }

  static bool decode(const Node& node, std::pair<First, Second>& tup) {
    if (!node.IsSequence() || node.size() != 2)
      return false;
    std::get<0>(tup) = node[0].template as<First>();
    std::get<1>(tup) = node[1].template as<Second>();
    return true;
  }
};

// binary
template <>
struct convert<Binary> {
  static Node encode(const Binary& rhs) {
    return Node(EncodeBase64(rhs.data(), rhs.size()));
  }

  static bool decode(const Node& node, Binary& rhs) {
    if (!node.IsScalar())
      return false;

    std::vector<unsigned char> data = DecodeBase64(node.Scalar());
    if (data.empty() && !node.Scalar().empty())
      return false;

    rhs.swap(data);
    return true;
  }
};
}

#endif  // NODE_CONVERT_H_62B23520_7C8E_11DE_8A39_0800200C9A66

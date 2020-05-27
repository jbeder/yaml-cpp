#ifndef STLEMITTER_H_62B23520_7C8E_11DE_8A39_0800200C9A66
#define STLEMITTER_H_62B23520_7C8E_11DE_8A39_0800200C9A66

#if defined(_MSC_VER) ||                                            \
    (defined(__GNUC__) && (__GNUC__ == 3 && __GNUC_MINOR__ >= 4) || \
     (__GNUC__ >= 4))  // GCC supports "pragma once" correctly since 3.4
#pragma once
#endif

#include <complex>
#include <sstream>
#include <tuple>
#include <type_traits>
#include <utility>

#include "emitter.h"

namespace YAML {

namespace detail {

template <bool P, typename T = void>
using enable_if_t = typename std::enable_if<P, T>::type;

template <typename... Args>
struct make_void {
  using type = void;
};

template <typename... Args>
using void_t = typename make_void<Args...>::type;

template <typename T, typename Test = void>
struct is_std_iterable : std::false_type {};

template <typename T>
struct is_std_iterable<
    T, enable_if_t<std::is_same<decltype(std::begin(std::declval<T>())),
                                decltype(std::end(std::declval<T>()))>::value>>
    : std::true_type {};

#define TRAITS_DECL_CLASS_HAS_TYPE(name)       \
  template <class T, typename Test = void>     \
  struct has_type_##name : std::false_type {}; \
  template <class T>                           \
  struct has_type_##name<T, void_t<typename T::name>> : std::true_type {};

TRAITS_DECL_CLASS_HAS_TYPE(element_type)
TRAITS_DECL_CLASS_HAS_TYPE(mapped_type)

#undef TRAITS_DECL_CLASS_HAS_TYPE

template <typename T>
inline enable_if_t<
    std::is_same<T, char>::value || std::is_same<T, unsigned char>::value, int>
    as_numeric(const T& value) {
  return static_cast<int>(value);
}

template <typename T>
inline enable_if_t<
    !(std::is_same<T, char>::value || std::is_same<T, unsigned char>::value), T>
    as_numeric(const T& value) {
  return value;
}

template <typename T, size_t N>
struct sequential_printer {
  template <typename S>
  inline static void print(S& stream, const T& value) {
    const auto i = N - 1;

    sequential_printer<T, i>::print(stream, value);
    stream << std::get<i>(value);
  }
};

template <typename T>
struct sequential_printer<T, 1> {
  template <typename S>
  inline static void print(S& stream, const T& value) {
    stream << std::get<0>(value);
  }
};

template <typename E, typename T>
inline E& emit_sequence(E& emitter, const T& value) {
  emitter << BeginSeq;
  for (const auto& item : value) {
    emitter << item;
  }
  return emitter << EndSeq;
}

template <typename E, typename T>
inline E& emit_mapping(E& emitter, const T& value) {
  emitter << BeginMap;
  for (const auto& key_value : value) {
    emitter << Key << std::get<0>(key_value) << Value << std::get<1>(key_value);
  }
  return emitter << EndMap;
}

template <typename T>
inline Emitter& emit_streamable(Emitter& emitter, const T& value,
                                std::stringstream* stream = nullptr) {
  std::stringstream stream_fallback;
  if (stream == nullptr) {
    stream = &stream_fallback;
  } else {
    stream->str("");
  }

  *stream << value;
  return emitter << stream->str();
}

template <typename T>
inline Emitter& emit_complex(Emitter& emitter, const T& real, const T& imag) {
#ifndef YAML_EMITTER_NO_COMPLEX

  std::stringstream ss;
  ss << as_numeric(real) << '+' << as_numeric(imag) << 'j';
  emitter << LocalTag("complex") << ss.str();
  return emitter;

#else

  return emitter << Flow << BeginSeq << as_numeric(real) << as_numeric(imag)
                 << EndSeq;

#endif
}

}  // namespace detail

//// std::complex

template <typename T>
inline Emitter& operator<<(Emitter& emitter, const std::complex<T>& value) {
  //	return detail::emit_streamable(emitter << LocalTag("complex"), value);
  //// stl style
  return detail::emit_complex(emitter, value.real(), value.imag());
}

//// std::pair

template <typename T1, typename T2>
inline Emitter& operator<<(Emitter& emitter, const std::pair<T1, T2>& value) {
  return emitter << Flow << BeginSeq << value.first << value.second << EndSeq;
}

//// std::tuple

template <typename... Args>
inline Emitter& operator<<(Emitter& emitter, const std::tuple<Args...>& value) {
  emitter << Flow << BeginSeq;
  detail::sequential_printer<std::tuple<Args...>, sizeof...(Args)>::print(
      emitter, value);
  return emitter << EndSeq;
}

//// std::array, std::vector, std::deque, std::list, std::forward_list
//// std::set, std::multiset, std::unordered_set

template <typename T>
inline detail::enable_if_t<detail::is_std_iterable<T>::value &&
                               !detail::has_type_mapped_type<T>::value,
                           Emitter&>
    operator<<(Emitter& emitter, const T& value) {
  return detail::emit_sequence(emitter, value);
}

//// std::map, std::unordered_map

template <typename T>
inline detail::enable_if_t<detail::is_std_iterable<T>::value &&
                               detail::has_type_mapped_type<T>::value,
                           Emitter&>
    operator<<(Emitter& emitter, const T& value) {
  return detail::emit_mapping(emitter, value);
}

//// std::unique_ptr, std::shared_ptr

template <typename T>
inline detail::enable_if_t<detail::has_type_element_type<T>::value, Emitter&>
    operator<<(Emitter& emitter, const T& value) {
  return emitter << value.get();
}

}  // namespace YAML

#endif  // STLEMITTER_H_62B23520_7C8E_11DE_8A39_0800200C9A66

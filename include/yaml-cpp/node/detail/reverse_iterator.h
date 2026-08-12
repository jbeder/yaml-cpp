#ifndef VALUE_DETAIL_REVERSE_ITERATOR_H_62B23520_7C8E_11DE_8A39_0800200C9A66
#define VALUE_DETAIL_REVERSE_ITERATOR_H_62B23520_7C8E_11DE_8A39_0800200C9A66




#if defined(_MSC_VER) ||                                            \
    (defined(__GNUC__) && (__GNUC__ == 3 && __GNUC_MINOR__ >= 4) || \
     (__GNUC__ >= 4))  // GCC supports "pragma once" correctly since 3.4
#pragma once


#endif

// IWYU pragma: private, include "yaml-cpp/yaml.h"
// IWYU pragma: friend "yaml-cpp/.*"


#include "yaml-cpp/dll.h"
#include "yaml-cpp/node/ptr.h"
#include <cstddef>
#include <iterator>

namespace YAML {
namespace detail {

// detail::iterator_base and detail::node_iterator_base are incompatible with
// std::reverse_iterator because their operator->() return a proxy object rather
// than a pointer, so a customized wrapper is needed to implement
// reverse_iterator for them.
template <typename Iter>
class reverse_iterator_base {
 private:
  Iter current;
  template <typename>
  friend class reverse_iterator_base;
  struct enabler {};
  using proxy_type = decltype(current.operator->());

 public:
  using iterator_type = Iter;
  using iterator_category = typename std::iterator_traits<Iter>::iterator_category;  
  using value_type = typename std::iterator_traits<Iter>::value_type;
  using difference_type = typename std::iterator_traits<Iter>::difference_type;
  using pointer = typename std::iterator_traits<Iter>::pointer;
  using reference = typename std::iterator_traits<Iter>::reference;

 public:
  reverse_iterator_base() : current() {}
  explicit reverse_iterator_base(Iter _base) : current(_base) {}
  template <class Iter2>
  reverse_iterator_base(const reverse_iterator_base<Iter2>& _other,
                typename std::enable_if<std::is_convertible<Iter2, Iter>::value,
                                        enabler>::type = enabler())
      : current(_other.current) {}
  
  reverse_iterator_base<Iter>& operator++() {
    --current;
    return *this;
  }

  reverse_iterator_base<Iter> operator++(int) {
    reverse_iterator_base<Iter> iterator_pre(*this);
    ++(*this);
    return iterator_pre;
  }
  
  reverse_iterator_base<Iter>& operator--() {
    ++current;
    return *this;
  }

  reverse_iterator_base<Iter> operator--(int) {
    reverse_iterator_base<Iter> iterator_pre(*this);
    --(*this);
    return iterator_pre;
  }

  template <typename Iter2>
  bool operator==(const reverse_iterator_base<Iter2>& rhs) const {
    return current == rhs.current;
  }

  template <typename Iter2>
  bool operator!=(const reverse_iterator_base<Iter2>& rhs) const {
    return current != rhs.current;
  }

  iterator_type base() const {
    return current;
  }

  value_type operator*() const YAML_ATTRIBUTE_LIFETIME_BOUND {
    Iter _tmp = current;
    return *(--_tmp);
  }

  proxy_type operator->() const YAML_ATTRIBUTE_LIFETIME_BOUND {
    Iter _tmp = current;
    --_tmp;
    return _tmp.operator->();
  }
};
}
}

#endif  // VALUE_DETAIL_NODE_ITERATOR_H_62B23520_7C8E_11DE_8A39_0800200C9A66

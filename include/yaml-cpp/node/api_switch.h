//
// Created by marcel on 3/3/22.
//

#ifndef YAML_CPP_API_SWITCH_H
#define YAML_CPP_API_SWITCH_H

#if defined(_MSC_VER) ||                                            \
    (defined(__GNUC__) && (__GNUC__ == 3 && __GNUC_MINOR__ >= 4) || \
     (__GNUC__ >= 4))  // GCC supports "pragma once" correctly since 3.4
#pragma once
#endif

#include "yaml-cpp/node/node.h"
#include "yaml-cpp/exceptions.h"
#include <type_traits>

namespace YAML{
namespace detail {

  //detect the method of the new api
  template <typename>
  std::false_type has_decode_new_api(long);

  template <typename T>
  auto has_decode_new_api(int)
      -> decltype( T::decode(std::declval<const Node&>()), std::true_type{});

  template <bool AorB>
  struct static_api_switch;

  template<> //new api call-path
  struct static_api_switch<true> {
    template<class T>
    static T decode(const Node& node) {
      return convert<T>::decode(node);
    }
  };

  template<>  //old api call-path
  struct static_api_switch<false> {
    template<class T>
    static T decode(const Node& node) {
      T t;
      if (convert<T>::decode(node, t))
        return t;
      throw conversion::DecodeException();
    }
  };
}
}

#endif  // YAML_CPP_API_SWITCH_H

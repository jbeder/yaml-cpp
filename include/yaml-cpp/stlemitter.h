#ifndef STLEMITTER_H_62B23520_7C8E_11DE_8A39_0800200C9A66
#define STLEMITTER_H_62B23520_7C8E_11DE_8A39_0800200C9A66

#if defined(_MSC_VER) ||                                            \
    (defined(__GNUC__) && (__GNUC__ == 3 && __GNUC_MINOR__ >= 4) || \
     (__GNUC__ >= 4))  // GCC supports "pragma once" correctly since 3.4
#pragma once
#endif

#include <vector>
#include <list>
#include <set>
#include <map>

namespace YAML {
template <typename Seq>
inline Emitter& EmitSeq(Emitter& emitter, const Seq& seq) {
  emitter << EMITTER_MANIP::BeginSeq;
  for (const auto& v : seq)
    emitter << v;
  emitter << EMITTER_MANIP::EndSeq;
  return emitter;
}

template <typename T>
inline Emitter& operator<<(Emitter& emitter, const std::vector<T>& v) {
  return EmitSeq(emitter, v);
}

template <typename T>
inline Emitter& operator<<(Emitter& emitter, const std::list<T>& v) {
  return EmitSeq(emitter, v);
}

template <typename T>
inline Emitter& operator<<(Emitter& emitter, const std::set<T>& v) {
  return EmitSeq(emitter, v);
}

template <typename K, typename V>
inline Emitter& operator<<(Emitter& emitter, const std::map<K, V>& m) {
  emitter << EMITTER_MANIP::BeginMap;
  for (const auto& v : m)
    emitter << EMITTER_MANIP::Key << v.first << EMITTER_MANIP::Value << v.second;
  emitter << EMITTER_MANIP::EndMap;
  return emitter;
}
}

#endif  // STLEMITTER_H_62B23520_7C8E_11DE_8A39_0800200C9A66

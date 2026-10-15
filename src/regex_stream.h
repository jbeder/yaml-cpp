#ifndef REGEX_STREAM_H_62B23520_7C8E_11DE_8A39_0800200C9A66
#define REGEX_STREAM_H_62B23520_7C8E_11DE_8A39_0800200C9A66

#if defined(_MSC_VER) ||                                            \
    (defined(__GNUC__) && (__GNUC__ == 3 && __GNUC_MINOR__ >= 4) || \
     (__GNUC__ >= 4))  // GCC supports "pragma once" correctly since 3.4
#pragma once
#endif

#include "regex_yaml.h"
#include "streamcharsource.h"

namespace YAML {
inline int Empty::Match(const StreamCharSource& source) {
  return static_cast<bool>(source) && source[0] == Stream::eof() ? 0 : -1;
}

template <typename Pattern>
struct StreamRegExInvoker {
  static int MatchStream(const Stream& stream) {
    return Pattern::Match(StreamCharSource(stream));
  }
};

template <typename Pattern>
constexpr RegEx MakeStreamRegEx() {
  return RegEx(&RegExInvoker<Pattern>::MatchString,
               &StreamRegExInvoker<Pattern>::MatchStream,
               &RegExInvoker<Pattern>::MatchChar);
}
}  // namespace YAML

#endif  // REGEX_STREAM_H_62B23520_7C8E_11DE_8A39_0800200C9A66

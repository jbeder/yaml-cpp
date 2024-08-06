#ifndef STREAMCHARSOURCE_H_62B23520_7C8E_11DE_8A39_0800200C9A66
#define STREAMCHARSOURCE_H_62B23520_7C8E_11DE_8A39_0800200C9A66

#if defined(_MSC_VER) ||                                            \
    (defined(__GNUC__) && (__GNUC__ == 3 && __GNUC_MINOR__ >= 4) || \
     (__GNUC__ >= 4))  // GCC supports "pragma once" correctly since 3.4
#pragma once
#endif

#include "stream.h"
#include "yaml-cpp/noexcept.h"
#include <cstddef>

namespace YAML {

class StreamCharSource {
 public:
  StreamCharSource(const Stream& stream) : m_offset(0), m_stream(stream) {}
  StreamCharSource(const StreamCharSource& source) = default;
  StreamCharSource(StreamCharSource&&) YAML_CPP_NOEXCEPT = default;
  StreamCharSource& operator=(const StreamCharSource&) = delete;
  StreamCharSource& operator=(StreamCharSource&&) = delete;
  ~StreamCharSource() = default;

  operator bool() const { return true; }

  char operator[](std::size_t i) const { return m_stream.peek(m_offset + i); }

  const StreamCharSource operator+(int i) const {
    StreamCharSource source(*this);
    if (static_cast<int>(source.m_offset) + i >= 0)
      source.m_offset += static_cast<std::size_t>(i);
    else
      source.m_offset = 0;
    return source;
  }

 private:
  std::size_t m_offset;
  const Stream& m_stream;
};

}  // namespace YAML

#endif  // STREAMCHARSOURCE_H_62B23520_7C8E_11DE_8A39_0800200C9A66

#ifndef STREAMCHARSOURCE_H_62B23520_7C8E_11DE_8A39_0800200C9A66
#define STREAMCHARSOURCE_H_62B23520_7C8E_11DE_8A39_0800200C9A66

#if defined(_MSC_VER) ||                                            \
    (defined(__GNUC__) && (__GNUC__ == 3 && __GNUC_MINOR__ >= 4) || \
     (__GNUC__ >= 4))  // GCC supports "pragma once" correctly since 3.4
#pragma once
#endif

#include "yaml-cpp/noncopyable.h"
#include <cstddef>

namespace YAML {
class StreamCharSource {
 public:
  StreamCharSource(const Stream& stream) : m_offset(0), m_stream(stream) {
    if (m_stream.ReadAheadTo(0)){
      m_char = m_stream.peek();
    } else {
      m_char = Stream::eof();
    }
  }
  ~StreamCharSource() {}

  inline operator bool() const { return m_char != Stream::eof(); }

  char operator[](std::size_t i) const { return m_stream.CharAt(m_offset + i); }

  char get() const { return m_char; }

  bool operator!() const { return !static_cast<bool>(*this); }

  const StreamCharSource operator+(int i) const {
    return StreamCharSource(
        *this, (static_cast<int>(m_offset) + i >= 0) ? m_offset + 1 : 0);
  }

 private:
  std::size_t m_offset;
  const Stream& m_stream;
  char m_char;

  StreamCharSource& operator=(const StreamCharSource&);  // non-assignable

  StreamCharSource(const StreamCharSource& source, size_t offset)
      : m_offset(offset), m_stream(source.m_stream) {

    if (m_stream.ReadAheadTo(m_offset)) {
      m_char = m_stream.CharAt(m_offset);
    } else {
      m_char = Stream::eof();
    }
  }
};
}

#endif  // STREAMCHARSOURCE_H_62B23520_7C8E_11DE_8A39_0800200C9A66

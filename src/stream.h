#ifndef STREAM_H_62B23520_7C8E_11DE_8A39_0800200C9A66
#define STREAM_H_62B23520_7C8E_11DE_8A39_0800200C9A66

#if defined(_MSC_VER) ||                                            \
    (defined(__GNUC__) && (__GNUC__ == 3 && __GNUC_MINOR__ >= 4) || \
     (__GNUC__ >= 4))  // GCC supports "pragma once" correctly since 3.4
#pragma once
#endif

#include "yaml-cpp/mark.h"
#include <cstddef>
#include <cstdint>
#include <deque>
#include <iostream>
#include <string>

namespace YAML {

// Converts arbitrary UTF-* encoding on input to UTF-8
class Stream {
 public:
  Stream(std::istream& input);
  Stream(const Stream&) = delete;
  Stream(Stream&&) = delete;
  Stream& operator=(const Stream&) = delete;
  Stream& operator=(Stream&&) = delete;
  ~Stream();

  static char eof() { return 0x04; }

  char peek(std::size_t i = 0) const;
  char get();
  std::string get(int n);
  void eat(int n = 1);

  bool isEmpty() const;
  operator bool() const;

  const Mark mark() const { return m_mark; }
  int pos() const { return m_mark.pos; }
  int line() const { return m_mark.line; }
  int column() const { return m_mark.column; }
  void ResetColumn() { m_mark.column = 0; }

 private:
  uint_fast8_t CheckBOM(const uint8_t* buffer, std::size_t size);
  bool prepare(std::size_t i) const;
  bool StreamInUtf8() const;
  bool StreamInUtf16() const;
  bool StreamInUtf32() const;
  bool GetNextByte(uint8_t& byte) const;

 private:
  std::istream& m_input;
  uint8_t* const m_pPrefetched;
  mutable std::size_t m_nPrefetchedAvailable;
  mutable std::size_t m_nPrefetchedUsed;

  mutable std::deque<char> m_readahead;
  Mark m_mark;

  enum { utf8, utf16le, utf16be, utf32le, utf32be } m_charSet;
};

}  // namespace YAML

#endif  // STREAM_H_62B23520_7C8E_11DE_8A39_0800200C9A66

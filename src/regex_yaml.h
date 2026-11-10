#ifndef REGEX_H_62B23520_7C8E_11DE_8A39_0800200C9A66
#define REGEX_H_62B23520_7C8E_11DE_8A39_0800200C9A66

#if defined(_MSC_VER) ||                                            \
    (defined(__GNUC__) && (__GNUC__ == 3 && __GNUC_MINOR__ >= 4) || \
     (__GNUC__ >= 4))  // GCC supports "pragma once" correctly since 3.4
#pragma once
#endif

#include <array>
#include <climits>
#include <cstddef>
#include <cstdint>
#include <string>

#include "stringsource.h"

namespace YAML {
class Stream;
class StreamCharSource;

namespace Detail {
static_assert(CHAR_BIT == 8, "CharBitSet requires 8-bit bytes (CHAR_BIT == 8)");

/**
 * @brief Fixed-capacity 256-bit set for byte-level character classes.
 *
 * Stores four 64-bit words in an std::array to provide constexpr bitwise
 * operations and constant-time membership testing without dynamic allocation.
 */
class CharBitSet {
 public:
  constexpr CharBitSet() : m_words{{0, 0, 0, 0}} {}

  constexpr CharBitSet operator|(const CharBitSet& rhs) const {
    return CharBitSet(m_words[0] | rhs.m_words[0], m_words[1] | rhs.m_words[1],
                      m_words[2] | rhs.m_words[2], m_words[3] | rhs.m_words[3]);
  }

  static constexpr CharBitSet FromByte(int byte) {
    return (byte < 0 || byte >= 256)
               ? CharBitSet()
               : CharBitSet(
                     (byte / 64 == 0) ? (std::uint64_t(1) << (byte % 64)) : 0,
                     (byte / 64 == 1) ? (std::uint64_t(1) << (byte % 64)) : 0,
                     (byte / 64 == 2) ? (std::uint64_t(1) << (byte % 64)) : 0,
                     (byte / 64 == 3) ? (std::uint64_t(1) << (byte % 64)) : 0);
  }

  constexpr bool Contains(unsigned char byte) const {
    return (m_words[byte / 64] & (std::uint64_t(1) << (byte % 64))) != 0;
  }

 private:
  constexpr CharBitSet(std::uint64_t w0, std::uint64_t w1, std::uint64_t w2,
                       std::uint64_t w3)
      : m_words{{w0, w1, w2, w3}} {}

  std::array<std::uint64_t, 4> m_words;
};

template <int... Bytes>
struct CharSetMask;

template <>
struct CharSetMask<> {
  static constexpr CharBitSet Value() { return CharBitSet(); }
};

template <int ByteVal, int... Bytes>
struct CharSetMask<ByteVal, Bytes...> {
  static constexpr CharBitSet Value() {
    return CharBitSet::FromByte(ByteVal) | CharSetMask<Bytes...>::Value();
  }
  static constexpr bool Contains(unsigned char byte) {
    return Value().Contains(byte);
  }
};
}  // namespace Detail

/**
 * @brief Pattern matching end of stream or empty input with zero length.
 */
struct Empty {
  static int Match(const StringCharSource& source) { return !source ? 0 : -1; }

  static int Match(const StreamCharSource& source);

  template <typename Source>
  static int Match(const Source& source) {
    return static_cast<bool>(source) ? -1 : 0;
  }

  static constexpr bool MatchesEmpty() { return true; }
  static constexpr bool MatchesOneChar(char) { return false; }
};

/**
 * @brief Pattern matching a single literal byte value.
 * @tparam ByteVal The byte value to match.
 */
template <int ByteVal>
struct Byte {
  template <typename Source>
  static int Match(const Source& source) {
    return static_cast<bool>(source) &&
                   static_cast<unsigned char>(source[0]) == ByteVal
               ? 1
               : -1;
  }

  static constexpr bool MatchesEmpty() { return false; }
  static constexpr bool MatchesOneChar(char ch) {
    return static_cast<unsigned char>(ch) == ByteVal;
  }
};

/**
 * @brief Pattern matching any single byte within inclusive range [First, Last].
 * @tparam First Lower bound of the byte range.
 * @tparam Last Upper bound of the byte range.
 */
template <int First, int Last>
struct Range {
  template <typename Source>
  static int Match(const Source& source) {
    return static_cast<bool>(source) &&
                   static_cast<unsigned char>(source[0]) >= First &&
                   static_cast<unsigned char>(source[0]) <= Last
               ? 1
               : -1;
  }

  static constexpr bool MatchesEmpty() { return false; }
  static constexpr bool MatchesOneChar(char ch) {
    return static_cast<unsigned char>(ch) >= First &&
           static_cast<unsigned char>(ch) <= Last;
  }
};

/**
 * @brief Pattern matching any byte from a compile-time set of byte values.
 * @tparam Bytes The variadic list of byte values forming the set.
 */
template <int... Bytes>
struct CharSet {
  template <typename Source>
  static int Match(const Source& source) {
    return static_cast<bool>(source) &&
                   Detail::CharSetMask<Bytes...>::Contains(
                       static_cast<unsigned char>(source[0]))
               ? 1
               : -1;
  }

  static constexpr bool MatchesEmpty() { return false; }
  static constexpr bool MatchesOneChar(char ch) {
    return Detail::CharSetMask<Bytes...>::Contains(
        static_cast<unsigned char>(ch));
  }
};

/**
 * @brief Alternation combinator matching the first matching pattern in order.
 * @tparam First First pattern to attempt.
 * @tparam Rest Subsequent fallback patterns.
 */
template <typename First, typename... Rest>
struct Or;

template <typename Pattern>
struct Or<Pattern> {
  template <typename Source>
  static int Match(const Source& source) {
    return Pattern::Match(source);
  }

  static constexpr bool MatchesEmpty() { return Pattern::MatchesEmpty(); }
  static constexpr bool MatchesOneChar(char ch) {
    return Pattern::MatchesOneChar(ch);
  }
};

template <typename First, typename Next, typename... Rest>
struct Or<First, Next, Rest...> {
  using Tail = Or<Next, Rest...>;

  template <typename Source>
  static int Match(const Source& source) {
    const int first = First::Match(source);
    return first >= 0 ? first : Tail::Match(source);
  }

  static constexpr bool MatchesEmpty() {
    return First::MatchesEmpty() || Tail::MatchesEmpty();
  }
  static constexpr bool MatchesOneChar(char ch) {
    return First::MatchesOneChar(ch) || Tail::MatchesOneChar(ch);
  }
};

/**
 * @brief Conjunction combinator requiring all patterns to match at position.
 * @tparam First First pattern that must match.
 * @tparam Rest Remaining patterns that must also match.
 */
template <typename First, typename... Rest>
struct And;

template <typename Pattern>
struct And<Pattern> {
  template <typename Source>
  static int Match(const Source& source) {
    return Pattern::Match(source);
  }

  static constexpr bool MatchesEmpty() { return Pattern::MatchesEmpty(); }
  static constexpr bool MatchesOneChar(char ch) {
    return Pattern::MatchesOneChar(ch);
  }
};

template <typename First, typename Next, typename... Rest>
struct And<First, Next, Rest...> {
  using Tail = And<Next, Rest...>;

  template <typename Source>
  static int Match(const Source& source) {
    const int first = First::Match(source);
    return first >= 0 && Tail::Match(source) >= 0 ? first : -1;
  }

  static constexpr bool MatchesEmpty() {
    return First::MatchesEmpty() && Tail::MatchesEmpty();
  }
  static constexpr bool MatchesOneChar(char ch) {
    return First::MatchesOneChar(ch) && Tail::MatchesOneChar(ch);
  }
};

/**
 * @brief Negation combinator matching one byte when Pattern fails to match.
 * @tparam Pattern The pattern that must not match.
 */
template <typename Pattern>
struct Not {
  template <typename Source>
  static int Match(const Source& source) {
    return Pattern::Match(source) >= 0 ? -1 : 1;
  }

  static constexpr bool MatchesEmpty() { return false; }
  static constexpr bool MatchesOneChar(char ch) {
    return !Pattern::MatchesOneChar(ch);
  }
};

/**
 * @brief Sequence combinator matching patterns consecutively from left to right.
 * @tparam First First pattern in the sequence.
 * @tparam Rest Subsequent patterns in the sequence.
 */
template <typename First, typename... Rest>
struct Seq;

template <typename Pattern>
struct Seq<Pattern> {
  template <typename Source>
  static int Match(const Source& source) {
    return Pattern::Match(source);
  }

  static constexpr bool MatchesEmpty() { return Pattern::MatchesEmpty(); }
  static constexpr bool MatchesOneChar(char ch) {
    return Pattern::MatchesOneChar(ch);
  }
};

template <typename First, typename Next, typename... Rest>
struct Seq<First, Next, Rest...> {
  using Tail = Seq<Next, Rest...>;

  template <typename Source>
  static int Match(const Source& source) {
    const int first = First::Match(source);
    if (first < 0)
      return -1;
    const int right = Tail::Match(source + first);
    return right < 0 ? -1 : first + right;
  }

  static constexpr bool MatchesEmpty() {
    return First::MatchesEmpty() && Tail::MatchesEmpty();
  }
  static constexpr bool MatchesOneChar(char ch) {
    return (First::MatchesEmpty() && Tail::MatchesOneChar(ch)) ||
           (First::MatchesOneChar(ch) && Tail::MatchesEmpty());
  }
};

template <typename Pattern>
struct RegExPattern {};

template <typename Pattern>
struct RegExInvoker {
  static int MatchString(const StringCharSource& source) {
    return Pattern::Match(source);
  }

  static constexpr bool MatchChar(char ch) {
    return Pattern::MatchesOneChar(ch);
  }
};

class RegEx;

template <typename Pattern>
constexpr RegEx MakeStreamRegEx();

/**
 * @brief Trivially destructible regular expression holding compiled matchers.
 *
 * Wraps compile-time template matchers behind uniform function pointers
 * without runtime heap allocations or dynamic AST construction.
 */
class RegEx {
 public:
  using StringMatcher = int (*)(const StringCharSource&);
  using StreamMatcher = int (*)(const Stream&);
  using CharMatcher = bool (*)(char);

  constexpr RegEx()
      : m_match_string(&RegExInvoker<Empty>::MatchString),
        m_match_stream(nullptr),
        m_match_char(&RegExInvoker<Empty>::MatchChar) {}

  template <typename Pattern>
  constexpr explicit RegEx(RegExPattern<Pattern>)
      : m_match_string(&RegExInvoker<Pattern>::MatchString),
        m_match_stream(nullptr),
        m_match_char(&RegExInvoker<Pattern>::MatchChar) {}

  int Match(const std::string& str) const {
    return Match(StringCharSource(str.c_str(), str.size()));
  }
  int Match(const StringCharSource& source) const {
    return m_match_string(source);
  }
  int Match(const Stream& stream) const {
    return m_match_stream ? m_match_stream(stream) : -1;
  }

  constexpr bool Matches(char ch) const { return m_match_char(ch); }
  bool Matches(const std::string& str) const { return Match(str) >= 0; }
  bool Matches(const StringCharSource& source) const {
    return Match(source) >= 0;
  }
  bool Matches(const Stream& stream) const { return Match(stream) >= 0; }

 private:
  constexpr RegEx(StringMatcher string_matcher, StreamMatcher stream_matcher,
                  CharMatcher char_matcher)
      : m_match_string(string_matcher),
        m_match_stream(stream_matcher),
        m_match_char(char_matcher) {}

  template <typename Pattern>
  friend constexpr RegEx MakeStreamRegEx();

  StringMatcher m_match_string;
  StreamMatcher m_match_stream;
  CharMatcher m_match_char;
};

template <typename Pattern>
constexpr RegEx MakeRegEx() {
  return RegEx(RegExPattern<Pattern>());
}
}  // namespace YAML

#endif  // REGEX_H_62B23520_7C8E_11DE_8A39_0800200C9A66

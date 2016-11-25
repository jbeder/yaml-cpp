#ifndef EXP_H_62B23520_7C8E_11DE_8A39_0800200C9A66
#define EXP_H_62B23520_7C8E_11DE_8A39_0800200C9A66

#if defined(_MSC_VER) ||                                            \
    (defined(__GNUC__) && (__GNUC__ == 3 && __GNUC_MINOR__ >= 4) || \
     (__GNUC__ >= 4))  // GCC supports "pragma once" correctly since 3.4
#pragma once
#endif

#include <ios>
#include <string>

#include "stream.h"
#include "stringsource.h"
#include "streamcharsource.h"

#define REGEXP_INLINE inline __attribute__((always_inline))
#define TEST_INLINE inline __attribute__((always_inline))
//#define TEST_INLINE __attribute__((noinline))

namespace YAML {

namespace Exp {

template <char N>
struct Char {
  template <typename Source>
  REGEXP_INLINE static int match(const Source& source) {
    return (source.get() == N) ? 1 : -1;
  }
};

template <typename A, typename... B>
struct OR {
  template <typename Source>
  REGEXP_INLINE static int match(const Source& source) {
    int pos = A::match(source);
    if (pos >= 0) {
      return pos;
    }

    return OR<B...>::match(source);
  }
};

template <typename A>
struct OR<A> {
  template <typename Source>
  REGEXP_INLINE static int match(const Source& source) {
    return A::match(source);
  }
};

template <typename A, typename... B>
struct SEQ {
  template <typename Source>
  REGEXP_INLINE static int match(const Source& source) {
    int a = A::match(source);
    if (a < 0) {
      return -1;
    }

    const Source nextSource = source + a;
    // if (nextSource) { c = nextSource[0]; }

    int b = SEQ<B...>::match(nextSource);
    if (b < 0) {
      return -1;
    }

    return a + b;
  }
};

template <typename A>
struct SEQ<A> {
  template <typename Source>
  REGEXP_INLINE static int match(const Source& source) {
    return A::match(source);
  }
};

// TODO empty???
template <typename A>
struct NOT {
  template <typename Source>
  REGEXP_INLINE static int match(const Source& source) {
    return A::match(source) >= 0 ? -1 : 1;
  }
};

template <char A, char Z>
struct Range {
  static_assert(A <= Z, "Invalid Range");
  template <typename Source>
  REGEXP_INLINE static int match(const Source& source) {
    return (source.get() < A || source.get() > Z) ? -1 : 1;
  }
};

struct Empty {
  template <typename Source>
  REGEXP_INLINE static int match(const Source& source) {
    return source.get() == Stream::eof() ? 0 : -1;
  }
  REGEXP_INLINE static int match(const StringCharSource& source) {
    // the empty regex only is successful on the empty string
    // return c == '\0' ? 0 : -1;
    return !source ? 0 : -1;
  }
};

template <typename Source>
inline bool IsValidSource(const Source& source) {
  return source;
}

template <>
inline bool IsValidSource<StringCharSource>(const StringCharSource& source) {
  // switch (m_op) {
  // case REGEX_MATCH:
  // case REGEX_RANGE:
  return source;
  // default:
  //     return true;
  // }
}

template <typename Exp>
struct Matcher {
  template <typename Source>
  TEST_INLINE static int Match(const Source& source) {
    // return IsValidSource(source) ? Exp::match(source, source[0]) : -1;
    return Exp::match(source);
  }

  template <typename Source>
  TEST_INLINE static bool Matches(const Source& source) {
    return Match(source) >= 0;
  }

  TEST_INLINE static int Match(const Stream& in) {
    StreamCharSource source(in);
    return Match(source);
  }
  TEST_INLINE static bool Matches(const Stream& in) {
    StreamCharSource source(in);
    return Matches(source);
  }

  TEST_INLINE static int Match(const std::string& str) {
    StringCharSource source(str.c_str(), str.size());
    return Match(source);
  }

  TEST_INLINE static bool Matches(const std::string& str) {
    return Match(str) >= 0;
  }

  TEST_INLINE static bool Matches(char ch) {
    std::string str;
    str += ch;
    return Matches(str);
  }
};

////////////////////////////////////////////////////////////////////////////////
// Here we store a bunch of expressions for matching different parts of the
// file.

namespace detail {

using Space = Char<' '>;

using Tab = Char<'\t'>;

using Blank = OR < Space, Tab >;

using Break =
  OR < Char<'\n'>,
       SEQ < Char<'\r'>,
             Char<'\n'> >>;

using BlankOrBreak = OR < Blank, Break >;

using Digit = Range<'0', '9'>;

using Alpha =
  OR < Range<'a', 'z'>,
       Range<'A', 'Z'> >;

using AlphaNumeric = OR < Alpha, Digit >;

using Word = OR < AlphaNumeric, Char<'-'> >;

using Hex = OR < Digit, Range<'a','f'>, Range<'A', 'F'>>;

// why not range?
using NotPrintable =
  OR < Char<0>, Char<'\x01'>,
       Char<'\x02'>, Char<'\x03'>,
       Char<'\x04'>, Char<'\x05'>,
       Char<'\x06'>, Char<'\x07'>,
       Char<'\x08'>, Char<'\x0B'>,
       Char<'\x0C'>, Char<'\x7F'>,
       Range<0x0E, 0x1F>,
       SEQ < Char<'\xC2'>,
             OR < Range<'\x80', '\x84'>,
                  Range<'\x86', '\x9F'>>>>;

using Utf8_ByteOrderMark =
  SEQ < Char<'\xEF'>,
        Char<'\xBB'>,
        Char<'\xBF'>>;

using DocStart =
  SEQ < Char<'-'>,
        Char<'-'>,
        Char<'-'>,
        OR < BlankOrBreak, Empty >>;

using DocEnd =
  SEQ < Char<'.'>,
        Char<'.'>,
        Char<'.'>,
        OR < BlankOrBreak, Empty>>;

using BlockEntry =
  SEQ < Char<'-'>,
        OR < BlankOrBreak, Empty >>;

using Key = SEQ<Char<'?'>, BlankOrBreak>;

using KeyInFlow = SEQ<Char<'?'>, BlankOrBreak>;

using Value =
  SEQ < Char<':'>,
        OR < BlankOrBreak, Empty >>;

using ValueInFlow =
  SEQ < Char<':'>,
        OR < BlankOrBreak,
             Char<','>,
             Char<'}'>>>;

using ValueInJSONFlow = Char<':'>;

using Comment = Char<'#'>;

using Anchor = NOT<
  OR < Char<'['>, Char<']'>,
       Char<'{'>, Char<'}'>,
       Char<','>,
       BlankOrBreak>>;

using AnchorEnd =
  OR < Char<'?'>, Char<':'>,
       Char<','>, Char<']'>,
       Char<'}'>, Char<'%'>,
       Char<'@'>, Char<'`'>,
       BlankOrBreak>;

using URI =
  OR < Word,
       Char<'#'>, Char<';'>, Char<'/'>, Char<'?'>, Char<':'>,
       Char<'@'>, Char<'&'>, Char<'='>, Char<'+'>, Char<'$'>,
       Char<','>, Char<'_'>, Char<'.'>, Char<'!'>, Char<'~'>,
       Char<'*'>, Char<'\''>, Char<'('>, Char<')'>, Char<'['>,
       Char<']'>,
       SEQ < Char<'%'>, Hex, Hex>>;

using Tag =
  OR < Word,
       Char<'#'>, Char<';'>, Char<'/'>, Char<'?'>, Char<':'>,
       Char<'@'>, Char<'&'>, Char<'='>, Char<'+'>, Char<'$'>,
       Char<'_'>, Char<'.'>, Char<'~'>, Char<'*'>, Char<'\''>,
       SEQ < Char <'%'>, Hex, Hex>>;

// Plain scalar rules:
// . Cannot start with a blank.
// . Can never start with any of , [ ] { } # & * ! | > \' \" % @ `
// . In the block context - ? : must be not be followed with a space.
// . In the flow context ? is illegal and : and - must not be followed with a
// space.
using PlainScalarCommon =
  NOT < OR < BlankOrBreak,
             Char<','>, Char<'['>, Char<']'>, Char<'{'>, Char<'}'>,
             Char<'#'>, Char<'&'>, Char<'*'>, Char<'!'>, Char<'|'>,
             Char<'>'>, Char<'\''>, Char<'\"'>, Char<'%'>, Char<'@'>,
             Char<'`'>>>;

using PlainScalar =
  NOT < SEQ < OR < Char<'-'>,
                   Char<'?'>,
                   Char<':'>>,
              OR < BlankOrBreak,
                   Empty >>>;

using PlainScalarInFlow =
  NOT < OR < Char<'?'>,
             SEQ < OR < Char<'-'>,
                        Char<':'>>,
                    Blank >>>;
using EndScalar =
  SEQ < Char<':'>,
        OR < BlankOrBreak, Empty >>;

using EndScalarInFlow =
  OR < SEQ < Char<':'>,
             OR < BlankOrBreak,
                  Empty,
                  Char<','>,
                  Char<']'>,
                  Char<'}'>>>,
       Char<','>,
       Char<'?'>,
       Char<'['>,
       Char<']'>,
       Char<'{'>,
       Char<'}'>>;



using ChompIndicator = OR < Char<'+'>, Char<'-'> >;

using Chomp =
  OR < SEQ < ChompIndicator, Digit >,
       SEQ < Digit,ChompIndicator >,
       ChompIndicator,
       Digit>;

} // end detail

using Tab = Matcher<detail::Tab>;
using Blank = Matcher<detail::Blank>;
using Break = Matcher<detail::Break>;
using Digit = Matcher<detail::Digit>;
using BlankOrBreak = Matcher<detail::BlankOrBreak>;
using Word = Matcher<detail::Word>;
using DocStart = Matcher<detail::DocStart>;
using DocEnd = Matcher<detail::DocEnd>;
using BlockEntry = Matcher<detail::BlockEntry>;
using Key = Matcher<detail::Key>;
using KeyInFlow = Matcher<detail::KeyInFlow>;
using Value = Matcher<detail::Value>;
using ValueInFlow = Matcher<detail::ValueInFlow>;
using ValueInJSONFlow = Matcher<detail::ValueInJSONFlow>;
using Comment = Matcher<detail::Comment>;
using Anchor = Matcher<detail::Anchor>;
using AnchorEnd = Matcher<detail::AnchorEnd>;
using URI = Matcher<detail::URI>;
using Tag = Matcher<detail::Tag>;
using PlainScalarCommon = Matcher<detail::PlainScalarCommon>;
using PlainScalar = Matcher<detail::PlainScalar>;
using PlainScalarInFlow = Matcher<detail::PlainScalarInFlow>;
using EscSingleQuote = Matcher<SEQ < Char<'\''>, Char<'\''> >>;
using EscBreak = Matcher<SEQ < Char<'\\'>, detail::Break >>;
using Chomp = Matcher<detail::Chomp>;

std::string Escape(Stream& in);
}

namespace Keys {
const char Directive = '%';
const char FlowSeqStart = '[';
const char FlowSeqEnd = ']';
const char FlowMapStart = '{';
const char FlowMapEnd = '}';
const char FlowEntry = ',';
const char Alias = '*';
const char Anchor = '&';
const char Tag = '!';
const char LiteralScalar = '|';
const char FoldedScalar = '>';
const char VerbatimTagStart = '<';
const char VerbatimTagEnd = '>';
}
}

#endif  // EXP_H_62B23520_7C8E_11DE_8A39_0800200C9A66

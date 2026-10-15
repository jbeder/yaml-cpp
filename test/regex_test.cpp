#include <sstream>
#include <type_traits>

#include "exp.h"
#include "regex_yaml.h"
#include "gtest/gtest.h"

using YAML::RegEx;

constexpr YAML::RegEx kSpaceExpression =
    YAML::MakeRegEx<YAML::Exp::Patterns::Space>();
static_assert(std::is_trivially_destructible<YAML::RegEx>::value,
              "RegEx must remain trivially destructible");
static_assert(std::is_trivially_destructible<YAML::Detail::CharBitSet>::value,
              "CharBitSet must remain trivially destructible");
static_assert(std::is_trivially_copyable<YAML::RegEx>::value,
              "RegEx must remain trivially copyable");
static_assert(kSpaceExpression.Matches(' '),
              "space must match itself at compile time");
static_assert(!kSpaceExpression.Matches('\t'),
              "space must not match tabs at compile time");
static_assert(YAML::Detail::CharBitSet::FromByte('a').Contains('a'),
              "CharBitSet must contain its initialized byte");
static_assert(!YAML::Detail::CharBitSet::FromByte('a').Contains('b'),
              "CharBitSet must not match uninitialized bytes");

namespace {
const auto MIN_CHAR = 0x04 + 1;

template <typename Pattern>
int Match(const std::string& input) {
  return YAML::MakeRegEx<Pattern>().Match(input);
}

TEST(RegExTest, Empty) {
  RegEx empty;
  EXPECT_TRUE(empty.Matches(std::string()));
  EXPECT_EQ(0, empty.Match(std::string()));
  for (int i = MIN_CHAR; i < 128; ++i) {
    auto str = std::string(1, char(i));
    EXPECT_FALSE(empty.Matches(str));
    EXPECT_EQ(-1, empty.Match(str));
  }
}

TEST(RegExTest, Byte) {
  auto ex = YAML::MakeRegEx<YAML::Byte<'a'>>();
  EXPECT_TRUE(ex.Matches('a'));
  EXPECT_FALSE(ex.Matches('b'));
  EXPECT_EQ(1, ex.Match(std::string("a")));
  EXPECT_EQ(1, ex.Match(std::string("ab")));
  EXPECT_EQ(-1, ex.Match(std::string("b")));
  EXPECT_EQ(-1, ex.Match(std::string()));
}

TEST(RegExTest, Range) {
  auto ex = YAML::MakeRegEx<YAML::Range<'a', 'z'>>();
  EXPECT_TRUE(ex.Matches('a'));
  EXPECT_TRUE(ex.Matches('z'));
  EXPECT_FALSE(ex.Matches('0'));
  for (int i = MIN_CHAR; i < 128; ++i) {
    auto str = std::string(1, char(i));
    if ('a' <= i && i <= 'z') {
      EXPECT_TRUE(ex.Matches(str));
      EXPECT_EQ(1, ex.Match(str));
    } else {
      EXPECT_FALSE(ex.Matches(str));
      EXPECT_EQ(-1, ex.Match(str));
    }
  }

  auto inverted = YAML::MakeRegEx<YAML::Range<'z', 'a'>>();
  EXPECT_FALSE(inverted.Matches('a'));
  EXPECT_FALSE(inverted.Matches(std::string("a")));
  EXPECT_EQ(-1, inverted.Match(std::string("a")));

  auto full_byte_range = YAML::MakeRegEx<YAML::Range<0, 255>>();
  EXPECT_TRUE(full_byte_range.Matches('a'));
  EXPECT_TRUE(full_byte_range.Matches('\0'));
  EXPECT_TRUE(full_byte_range.Matches('\xFF'));
  EXPECT_EQ(1, full_byte_range.Match(std::string("abc")));
}

TEST(RegExTest, CharSet) {
  auto ex = YAML::MakeRegEx<YAML::CharSet<'a', 'e', 'i', 'o', 'u'>>();
  EXPECT_TRUE(ex.Matches('a'));
  EXPECT_TRUE(ex.Matches('e'));
  EXPECT_TRUE(ex.Matches('i'));
  EXPECT_TRUE(ex.Matches('o'));
  EXPECT_TRUE(ex.Matches('u'));
  EXPECT_FALSE(ex.Matches('b'));
  EXPECT_FALSE(ex.Matches(' '));
  EXPECT_EQ(1, ex.Match(std::string("apple")));
  EXPECT_EQ(-1, ex.Match(std::string("banana")));
}

TEST(RegExTest, Seq) {
  auto ex = YAML::MakeRegEx<YAML::Seq<YAML::Byte<'a'>, YAML::Byte<'b'>>>();
  EXPECT_FALSE(ex.Matches('a'));
  EXPECT_FALSE(ex.Matches(std::string("a")));
  EXPECT_EQ(-1, ex.Match(std::string("a")));
  EXPECT_TRUE(ex.Matches(std::string("ab")));
  EXPECT_EQ(2, ex.Match(std::string("ab")));
  EXPECT_TRUE(ex.Matches(std::string("abba")));
  EXPECT_EQ(2, ex.Match(std::string("abba")));
  EXPECT_FALSE(ex.Matches(std::string("baab")));
  EXPECT_EQ(-1, ex.Match(std::string("baab")));

  auto seq_with_empty =
      YAML::MakeRegEx<YAML::Seq<YAML::Byte<'a'>, YAML::Empty>>();
  EXPECT_TRUE(seq_with_empty.Matches('a'));
  EXPECT_FALSE(seq_with_empty.Matches('b'));

  auto variadic_seq = YAML::MakeRegEx<
      YAML::Seq<YAML::Byte<'a'>, YAML::Byte<'b'>, YAML::Byte<'c'>>>();
  EXPECT_TRUE(variadic_seq.Matches(std::string("abc")));
  EXPECT_EQ(3, variadic_seq.Match(std::string("abc")));
  EXPECT_FALSE(variadic_seq.Matches(std::string("ab")));
  EXPECT_EQ(-1, variadic_seq.Match(std::string("ab")));
}

TEST(RegExTest, Or) {
  auto ex = YAML::MakeRegEx<YAML::Or<YAML::Byte<'a'>, YAML::Byte<'b'>>>();
  EXPECT_TRUE(ex.Matches('a'));
  EXPECT_TRUE(ex.Matches('b'));
  EXPECT_FALSE(ex.Matches('c'));
  EXPECT_EQ(1, ex.Match(std::string("a")));
  EXPECT_EQ(1, ex.Match(std::string("b")));
  EXPECT_EQ(-1, ex.Match(std::string("c")));

  auto variadic_or = YAML::MakeRegEx<
      YAML::Or<YAML::Byte<'a'>, YAML::Byte<'b'>, YAML::Byte<'c'>>>();
  EXPECT_TRUE(variadic_or.Matches('a'));
  EXPECT_TRUE(variadic_or.Matches('b'));
  EXPECT_TRUE(variadic_or.Matches('c'));
  EXPECT_FALSE(variadic_or.Matches('d'));
  EXPECT_EQ(1, variadic_or.Match(std::string("c")));

  using QuadA = YAML::Seq<YAML::Byte<'a'>, YAML::Byte<'a'>, YAML::Byte<'a'>,
                          YAML::Byte<'a'>>;
  using DoubleA = YAML::Seq<YAML::Byte<'a'>, YAML::Byte<'a'>>;
  auto or_quad_first = YAML::MakeRegEx<YAML::Or<QuadA, DoubleA>>();
  auto or_double_first = YAML::MakeRegEx<YAML::Or<DoubleA, QuadA>>();
  EXPECT_EQ(4, or_quad_first.Match(std::string("aaaaa")));
  EXPECT_EQ(2, or_double_first.Match(std::string("aaaaa")));
}

TEST(RegExTest, And) {
  auto ex =
      YAML::MakeRegEx<YAML::And<YAML::Range<'a', 'z'>, YAML::Byte<'m'>>>();
  EXPECT_TRUE(ex.Matches('m'));
  EXPECT_FALSE(ex.Matches('a'));
  EXPECT_FALSE(ex.Matches('z'));
  EXPECT_EQ(1, ex.Match(std::string("m")));
  EXPECT_EQ(-1, ex.Match(std::string("a")));

  auto variadic_and = YAML::MakeRegEx<YAML::And<
      YAML::Range<'a', 'z'>, YAML::Range<'k', 'z'>, YAML::Byte<'m'>>>();
  EXPECT_TRUE(variadic_and.Matches('m'));
  EXPECT_FALSE(variadic_and.Matches('a'));
  EXPECT_FALSE(variadic_and.Matches('k'));

  using QuadA = YAML::Seq<YAML::Byte<'a'>, YAML::Byte<'a'>, YAML::Byte<'a'>,
                          YAML::Byte<'a'>>;
  using DoubleA = YAML::Seq<YAML::Byte<'a'>, YAML::Byte<'a'>>;
  auto and_quad_first = YAML::MakeRegEx<YAML::And<QuadA, DoubleA>>();
  auto and_double_first = YAML::MakeRegEx<YAML::And<DoubleA, QuadA>>();
  EXPECT_EQ(4, and_quad_first.Match(std::string("aaaaa")));
  EXPECT_EQ(2, and_double_first.Match(std::string("aaaaa")));

  auto and_mismatch = YAML::MakeRegEx<YAML::And<DoubleA, YAML::Byte<'b'>>>();
  EXPECT_EQ(-1, and_mismatch.Match(std::string("aaaaa")));
}

TEST(RegExTest, Not) {
  auto ex = YAML::MakeRegEx<YAML::Not<YAML::Byte<'a'>>>();
  EXPECT_FALSE(ex.Matches('a'));
  EXPECT_TRUE(ex.Matches('b'));
  EXPECT_EQ(-1, ex.Match(std::string("a")));
  EXPECT_EQ(1, ex.Match(std::string("b")));
  EXPECT_EQ(1, ex.Match(std::string("ba")));

  using AB = YAML::Seq<YAML::Byte<'a'>, YAML::Byte<'b'>>;
  auto not_ab = YAML::MakeRegEx<YAML::Not<AB>>();
  EXPECT_TRUE(not_ab.Matches(std::string("a")));
  EXPECT_EQ(1, not_ab.Match(std::string("a")));
  EXPECT_FALSE(not_ab.Matches(std::string("ab")));
  EXPECT_EQ(-1, not_ab.Match(std::string("ab")));
  EXPECT_FALSE(not_ab.Matches(std::string("abba")));
  EXPECT_EQ(-1, not_ab.Match(std::string("abba")));
  EXPECT_TRUE(not_ab.Matches(std::string("baab")));
  EXPECT_EQ(1, not_ab.Match(std::string("baab")));
}

TEST(RegExTest, BitSetBoundaryAndByteOrder) {
  auto bitset =
      YAML::Detail::CharSetMask<0, 63, 64, 127, 128, 191, 192, 255>::Value();
  EXPECT_TRUE(bitset.Contains(0));
  EXPECT_TRUE(bitset.Contains(63));
  EXPECT_TRUE(bitset.Contains(64));
  EXPECT_TRUE(bitset.Contains(127));
  EXPECT_TRUE(bitset.Contains(128));
  EXPECT_TRUE(bitset.Contains(191));
  EXPECT_TRUE(bitset.Contains(192));
  EXPECT_TRUE(bitset.Contains(255));
  EXPECT_FALSE(bitset.Contains(1));
  EXPECT_FALSE(bitset.Contains(62));
  EXPECT_FALSE(bitset.Contains(65));
  EXPECT_FALSE(bitset.Contains(126));
  EXPECT_FALSE(bitset.Contains(129));
  EXPECT_FALSE(bitset.Contains(190));
  EXPECT_FALSE(bitset.Contains(193));
  EXPECT_FALSE(bitset.Contains(254));
}

TEST(RegExTest, FixedPatternsMatchBoundaries) {
  EXPECT_EQ(3, Match<YAML::Exp::Patterns::Tag>(std::string("%21")));
  EXPECT_EQ(3, Match<YAML::Exp::Patterns::Uri>(std::string("%21")));
  EXPECT_EQ(4, Match<YAML::Exp::Patterns::DocumentEnd>(std::string("...\n")));
  EXPECT_EQ(-1, Match<YAML::Exp::Patterns::ValueInFlow>(std::string(":")));
  EXPECT_EQ(2, Match<YAML::Exp::Patterns::ValueInFlow>(std::string(":,")));
}

TEST(RegExTest, FixedPatternsCoverScalarBoundaries) {
  EXPECT_EQ(1, Match<YAML::Exp::Patterns::PlainScalar>(std::string("a")));
  EXPECT_EQ(-1, Match<YAML::Exp::Patterns::PlainScalar>(std::string(":")));
  EXPECT_EQ(1, Match<YAML::Exp::Patterns::PlainScalarInFlow>(std::string("a")));
  EXPECT_EQ(-1,
            Match<YAML::Exp::Patterns::PlainScalarInFlow>(std::string(":")));

  EXPECT_EQ(1, Match<YAML::Exp::Patterns::SingleQuoteEnd>(std::string("'")));
  EXPECT_EQ(-1, Match<YAML::Exp::Patterns::SingleQuoteEnd>(std::string("''")));
  EXPECT_EQ(1, Match<YAML::Exp::Patterns::DoubleQuoteEnd>(std::string("\"")));
  EXPECT_EQ(-1, Match<YAML::Exp::Patterns::DoubleQuoteEnd>(std::string("x")));

  EXPECT_EQ(1, Match<YAML::Exp::Patterns::NotPrintable>(std::string("\x01")));
  EXPECT_EQ(1, Match<YAML::Exp::Patterns::DisallowedBlock>(std::string("\t")));
  EXPECT_EQ(3, Match<YAML::Exp::Patterns::DisallowedBlock>(
                   std::string("\xEF\xBB\xBF", 3)));
  EXPECT_EQ(-1, Match<YAML::Exp::Patterns::DisallowedBlock>(std::string("a")));
}

TEST(RegExTest, NegatedPatternsPreserveEmptySourceBoundary) {
  EXPECT_EQ(1, Match<YAML::Exp::Patterns::PlainScalar>(std::string()));
  EXPECT_EQ(1, Match<YAML::Exp::Patterns::PlainScalarInFlow>(std::string()));
  EXPECT_EQ(1, Match<YAML::Exp::Patterns::Anchor>(std::string()));
}

TEST(RegExTest, EmptyPatternMatchesStringEnd) {
  std::string input("...");
  YAML::StringCharSource source(input.c_str(), input.size());

  EXPECT_EQ(3, Match<YAML::Exp::Patterns::DocumentEnd>(input));
  EXPECT_EQ(0, YAML::Empty::Match(source + 3));
}
}  // namespace

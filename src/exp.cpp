#include <sstream>

#include "exp.h"
#include "regex_stream.h"
#include "stream.h"
#include "yaml-cpp/exceptions.h"  // IWYU pragma: keep

namespace YAML {
struct Mark;
}  // namespace YAML

namespace YAML {
namespace Exp {
namespace {
template <typename Pattern>
const RegEx& Matcher() {
  static constexpr RegEx expression = MakeStreamRegEx<Pattern>();
  return expression;
}
}  // namespace

const RegEx& Empty() { return Matcher<Patterns::Empty>(); }
const RegEx& Space() { return Matcher<Patterns::Space>(); }
const RegEx& Tab() { return Matcher<Patterns::Tab>(); }
const RegEx& Blank() { return Matcher<Patterns::Blank>(); }
const RegEx& Break() { return Matcher<Patterns::Break>(); }
const RegEx& BlankOrBreak() { return Matcher<Patterns::BlankOrBreak>(); }
const RegEx& Digit() { return Matcher<Patterns::Digit>(); }
const RegEx& Alpha() { return Matcher<Patterns::Alpha>(); }
const RegEx& AlphaNumeric() { return Matcher<Patterns::AlphaNumeric>(); }
const RegEx& Word() { return Matcher<Patterns::Word>(); }
const RegEx& Hex() { return Matcher<Patterns::Hex>(); }
const RegEx& NotPrintable() { return Matcher<Patterns::NotPrintable>(); }
const RegEx& Utf8_ByteOrderMark() {
  return Matcher<Patterns::Utf8ByteOrderMark>();
}
const RegEx& DocStart() { return Matcher<Patterns::DocumentStart>(); }
const RegEx& DocEnd() { return Matcher<Patterns::DocumentEnd>(); }
const RegEx& DocIndicator() { return Matcher<Patterns::DocumentIndicator>(); }
const RegEx& BlockEntry() { return Matcher<Patterns::BlockEntry>(); }
const RegEx& Key() { return Matcher<Patterns::Key>(); }
const RegEx& KeyInFlow() { return Matcher<Patterns::KeyInFlow>(); }
const RegEx& Value() { return Matcher<Patterns::Value>(); }
const RegEx& ValueInFlow() { return Matcher<Patterns::ValueInFlow>(); }
const RegEx& ValueInJSONFlow() { return Matcher<Patterns::ValueInJSONFlow>(); }
const RegEx& Ampersand() { return Matcher<Patterns::Ampersand>(); }
const RegEx& Comment() { return Matcher<Patterns::Comment>(); }
const RegEx& Anchor() { return Matcher<Patterns::Anchor>(); }
const RegEx& AnchorEnd() { return Matcher<Patterns::AnchorEnd>(); }
const RegEx& URI() { return Matcher<Patterns::Uri>(); }
const RegEx& Tag() { return Matcher<Patterns::Tag>(); }
const RegEx& PlainScalar() { return Matcher<Patterns::PlainScalar>(); }
const RegEx& PlainScalarInFlow() {
  return Matcher<Patterns::PlainScalarInFlow>();
}
const RegEx& EndScalar() { return Matcher<Patterns::EndScalar>(); }
const RegEx& EndScalarInFlow() { return Matcher<Patterns::EndScalarInFlow>(); }
const RegEx& ScanScalarEndInFlow() {
  return Matcher<Patterns::ScanScalarEndInFlow>();
}
const RegEx& ScanScalarEnd() { return Matcher<Patterns::ScanScalarEnd>(); }
const RegEx& EscSingleQuote() { return Matcher<Patterns::EscSingleQuote>(); }
const RegEx& EscBreak() { return Matcher<Patterns::EscBreak>(); }
const RegEx& SingleQuoteEnd() { return Matcher<Patterns::SingleQuoteEnd>(); }
const RegEx& DoubleQuoteEnd() { return Matcher<Patterns::DoubleQuoteEnd>(); }
const RegEx& ChompIndicator() { return Matcher<Patterns::ChompIndicator>(); }
const RegEx& Chomp() { return Matcher<Patterns::Chomp>(); }
const RegEx& DisallowedFlow() { return Matcher<Patterns::DisallowedFlow>(); }
const RegEx& DisallowedBlock() { return Matcher<Patterns::DisallowedBlock>(); }

unsigned ParseHex(const std::string& str, const Mark& mark) {
  unsigned value = 0;
  for (char ch : str) {
    int digit = 0;
    if ('a' <= ch && ch <= 'f')
      digit = ch - 'a' + 10;
    else if ('A' <= ch && ch <= 'F')
      digit = ch - 'A' + 10;
    else if ('0' <= ch && ch <= '9')
      digit = ch - '0';
    else
      throw ParserException(mark, ErrorMsg::INVALID_HEX);

    value = (value << 4) + digit;
  }

  return value;
}

std::string Str(unsigned ch) { return std::string(1, static_cast<char>(ch)); }

// Escape
// . Translates the next 'codeLength' characters into a hex number and returns
// the result.
// . Throws if it's not actually hex.
std::string Escape(Stream& in, int codeLength) {
  // grab string
  std::string str;
  for (int i = 0; i < codeLength; i++)
    str += in.get();

  // get the value
  unsigned value = ParseHex(str, in.mark());

  // legal unicode?
  if ((value >= 0xD800 && value <= 0xDFFF) || value > 0x10FFFF) {
    std::stringstream msg;
    msg << ErrorMsg::INVALID_UNICODE << value;
    throw ParserException(in.mark(), msg.str());
  }

  // now break it up into chars
  if (value <= 0x7F)
    return Str(value);

  if (value <= 0x7FF)
    return Str(0xC0 + (value >> 6)) + Str(0x80 + (value & 0x3F));

  if (value <= 0xFFFF)
    return Str(0xE0 + (value >> 12)) + Str(0x80 + ((value >> 6) & 0x3F)) +
           Str(0x80 + (value & 0x3F));

  return Str(0xF0 + (value >> 18)) + Str(0x80 + ((value >> 12) & 0x3F)) +
         Str(0x80 + ((value >> 6) & 0x3F)) + Str(0x80 + (value & 0x3F));
}

// Escape
// . Escapes the sequence starting 'in' (it must begin with a '\' or single
// quote)
//   and returns the result.
// . Throws if it's an unknown escape character.
std::string Escape(Stream& in) {
  // eat slash
  char escape = in.get();

  // switch on escape character
  char ch = in.get();

  // first do single quote, since it's easier
  if (escape == '\'' && ch == '\'')
    return "\'";

  // now do the slash (we're not gonna check if it's a slash - you better pass
  // one!)
  switch (ch) {
    case '0':
      return std::string(1, '\x00');
    case 'a':
      return "\x07";
    case 'b':
      return "\x08";
    case 't':
    case '\t':
      return "\x09";
    case 'n':
      return "\x0A";
    case 'v':
      return "\x0B";
    case 'f':
      return "\x0C";
    case 'r':
      return "\x0D";
    case 'e':
      return "\x1B";
    case ' ':
      return R"( )";
    case '\"':
      return "\"";
    case '\'':
      return "\'";
    case '\\':
      return "\\";
    case '/':
      return "/";
    case 'N':
      return "\xC2\x85";      // NEL (U+0085)
    case '_':
      return "\xC2\xA0";      // NBSP (U+00A0)
    case 'L':
      return "\xE2\x80\xA8";  // LS (U+2028)
    case 'P':
      return "\xE2\x80\xA9";  // PS (U+2029)
    case 'x':
      return Escape(in, 2);
    case 'u':
      return Escape(in, 4);
    case 'U':
      return Escape(in, 8);
  }

  std::stringstream msg;
  throw ParserException(in.mark(), std::string(ErrorMsg::INVALID_ESCAPE) + ch);
}
}  // namespace Exp
}  // namespace YAML

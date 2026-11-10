#ifndef EXP_H_62B23520_7C8E_11DE_8A39_0800200C9A66
#define EXP_H_62B23520_7C8E_11DE_8A39_0800200C9A66

#if defined(_MSC_VER) ||                                            \
    (defined(__GNUC__) && (__GNUC__ == 3 && __GNUC_MINOR__ >= 4) || \
     (__GNUC__ >= 4))  // GCC supports "pragma once" correctly since 3.4
#pragma once
#endif

#include <ios>
#include <string>

#include "regex_yaml.h"

namespace YAML {
class Stream;
////////////////////////////////////////////////////////////////////////////////
// Here we store a bunch of expressions for matching different parts of the
// file.

namespace Exp {
namespace Patterns {
using Empty = YAML::Empty;
using Space = CharSet<' '>;
using Tab = CharSet<'\t'>;
using Blank = CharSet<' ', '\t'>;
using LineFeed = Byte<'\n'>;
using CarriageReturnLineFeed = Seq<Byte<'\r'>, Byte<'\n'>>;
using Break = Or<LineFeed, CarriageReturnLineFeed, Byte<'\r'>>;
using BlankOrBreak = Or<Blank, Break>;
using Digit = Range<'0', '9'>;
using Alpha = Or<Range<'a', 'z'>, Range<'A', 'Z'>>;
using AlphaNumeric = Or<Alpha, Digit>;
using Word = Or<AlphaNumeric, Byte<'-'>>;
using Hex = Or<Digit, Range<'A', 'F'>, Range<'a', 'f'>>;

using NotPrintableBytes =
    CharSet<'\x01', '\x02', '\x03', '\x04', '\x05', '\x06', '\a', '\b', '\v',
            '\f', '\x7F'>;
using NotPrintableUtf8Tail = Or<Range<0x80, 0x84>, Range<0x86, 0x9F>>;
using NotPrintableUtf8 = Seq<Byte<0xC2>, NotPrintableUtf8Tail>;
using NotPrintable =
    Or<Byte<0>, NotPrintableBytes, Range<0x0E, 0x1F>, NotPrintableUtf8>;
using Utf8ByteOrderMark = Seq<Byte<0xEF>, Byte<0xBB>, Byte<0xBF>>;

using DocumentStartPrefix = Seq<Byte<'-'>, Byte<'-'>, Byte<'-'>>;
using DocumentEndPrefix = Seq<Byte<'.'>, Byte<'.'>, Byte<'.'>>;
using DocumentMarkerSuffix = Or<BlankOrBreak, Empty>;
using DocumentStart = Seq<DocumentStartPrefix, DocumentMarkerSuffix>;
using DocumentEnd = Seq<DocumentEndPrefix, DocumentMarkerSuffix>;
using DocumentIndicator = Or<DocumentStart, DocumentEnd>;
using BlockEntry = Seq<Byte<'-'>, DocumentMarkerSuffix>;
using Key = Seq<Byte<'?'>, BlankOrBreak>;
using KeyInFlow = Key;
using Value = Seq<Byte<':'>, DocumentMarkerSuffix>;
using FlowValueTerminators = CharSet<',', ']', '}'>;
using ValueInFlow = Seq<Byte<':'>, Or<BlankOrBreak, FlowValueTerminators>>;
using ValueInJSONFlow = Byte<':'>;
using Ampersand = Byte<'&'>;
using Comment = Byte<'#'>;
using AnchorTerminators = CharSet<'[', ']', '{', '}', ','>;
using Anchor = Not<Or<AnchorTerminators, BlankOrBreak>>;
using AnchorEnd =
    Or<CharSet<'?', ':', ',', ']', '}', '%', '@', 0x60>, BlankOrBreak>;

using UriCharacters =
    CharSet<'#', ';', '/', '?', ':', '@', '&', '=', '+', '$', ',', '_', '.',
            '!', '~', '*', '\'', '(', ')', '[', ']'>;
using TagCharacters = CharSet<'#', ';', '/', '?', ':', '@', '&', '=', '+', '$',
                              '_', '.', '~', '*', '\'', '(', ')'>;
using PercentEncoded = Seq<Byte<'%'>, Hex, Hex>;
using Uri = Or<Word, UriCharacters, PercentEncoded>;
using Tag = Or<Word, TagCharacters, PercentEncoded>;

using PlainScalarTerminators =
    CharSet<',', '[', ']', '{', '}', '#', '&', '*', '!', '|', '>', '\'', '"',
            '%', '@', 0x60>;
using PlainScalarIndicators = CharSet<'-', '?', ':'>;
using PlainScalarRejection =
    Or<BlankOrBreak, PlainScalarTerminators,
       Seq<PlainScalarIndicators, Or<BlankOrBreak, Empty>>>;
using PlainScalar = Not<PlainScalarRejection>;

using PlainScalarFlowTerminators =
    CharSet<'?', ',', '[', ']', '{', '}', '#', '&', '*', '!', '|', '>', '\'',
            '"', '%', '@', 0x60>;
using PlainScalarFlowIndicators = CharSet<'-', ':'>;
using PlainScalarFlowRejection =
    Or<BlankOrBreak, PlainScalarFlowTerminators,
       Seq<PlainScalarFlowIndicators, Or<Blank, Empty>>>;
using PlainScalarInFlow = Not<PlainScalarFlowRejection>;

using EndScalar = Seq<Byte<':'>, DocumentMarkerSuffix>;
using FlowScalarTerminators = CharSet<',', '?', '[', ']', '{', '}'>;
using EndScalarInFlowPrefix =
    Seq<Byte<':'>, Or<BlankOrBreak, Empty, FlowValueTerminators>>;
using EndScalarInFlow = Or<EndScalarInFlowPrefix, FlowScalarTerminators>;
using CommentAfterBreak = Seq<BlankOrBreak, Comment>;
using ScanScalarEndInFlow = Or<EndScalarInFlow, CommentAfterBreak>;
using ScanScalarEnd = Or<EndScalar, CommentAfterBreak>;
using EscSingleQuote = Seq<Byte<'\''>, Byte<'\''>>;
using EscBreak = Seq<Byte<'\\'>, Break>;
using SingleQuoteEnd = And<Byte<'\''>, Not<EscSingleQuote>>;
using DoubleQuoteEnd = Byte<'"'>;
using ChompIndicator = CharSet<'+', '-'>;
using Chomp = Or<Seq<ChompIndicator, Digit>, Seq<Digit, ChompIndicator>,
                 ChompIndicator, Digit>;
using DisallowedWhitespace = Or<Tab, Ampersand>;
using DisallowedBreak = Or<Break, DisallowedWhitespace>;
using DisallowedEncoding = Or<Utf8ByteOrderMark, DisallowedBreak>;
using DisallowedCharacters = Or<NotPrintable, DisallowedEncoding>;
using DisallowedAfterComment = Or<CommentAfterBreak, DisallowedCharacters>;
using DisallowedFlow = Or<EndScalarInFlow, DisallowedAfterComment>;
using DisallowedBlock = Or<EndScalar, DisallowedAfterComment>;
}  // namespace Patterns

const RegEx& Empty();
const RegEx& Space();
const RegEx& Tab();
const RegEx& Blank();
const RegEx& Break();
const RegEx& BlankOrBreak();
const RegEx& Digit();
const RegEx& Alpha();
const RegEx& AlphaNumeric();
const RegEx& Word();
const RegEx& Hex();
const RegEx& NotPrintable();
const RegEx& Utf8_ByteOrderMark();
const RegEx& DocStart();
const RegEx& DocEnd();
const RegEx& DocIndicator();
const RegEx& BlockEntry();
const RegEx& Key();
const RegEx& KeyInFlow();
const RegEx& Value();
const RegEx& ValueInFlow();
const RegEx& ValueInJSONFlow();
const RegEx& Ampersand();
const RegEx& Comment();
const RegEx& Anchor();
const RegEx& AnchorEnd();
const RegEx& URI();
const RegEx& Tag();
const RegEx& PlainScalar();
const RegEx& PlainScalarInFlow();
const RegEx& EndScalar();
const RegEx& EndScalarInFlow();
const RegEx& ScanScalarEndInFlow();
const RegEx& ScanScalarEnd();
const RegEx& EscSingleQuote();
const RegEx& EscBreak();
const RegEx& SingleQuoteEnd();
const RegEx& DoubleQuoteEnd();
const RegEx& ChompIndicator();
const RegEx& Chomp();
const RegEx& DisallowedFlow();
const RegEx& DisallowedBlock();

// and some functions
std::string Escape(Stream& in);
}  // namespace Exp

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
}  // namespace Keys
}  // namespace YAML

#endif  // EXP_H_62B23520_7C8E_11DE_8A39_0800200C9A66

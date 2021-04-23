#include <sstream>

#include "exp.h"
#include "regex_yaml.h"
#include "regeximpl.h"
#include "scanner.h"
#include "scanscalar.h"
#include "scantag.h"  // IWYU pragma: keep
#include "tag.h"      // IWYU pragma: keep
#include "token.h"
#include "yaml-cpp/exceptions.h"  // IWYU pragma: keep
#include "yaml-cpp/mark.h"

namespace YAML {
///////////////////////////////////////////////////////////////////////
// Specialization for scanning specific tokens

// Directive
// . Note: no semantic checking is done here (that's for the parser to do)
void Scanner::ScanDirective() {
  std::string name;
  std::vector<std::string> params;

  // pop indents and simple keys
  PopAllIndents();
  PopAllSimpleKeys();

  m_simpleKeyAllowed = false;
  m_canBeJSONFlow = false;

  // store pos and eat indicator
  Token token(Token::TYPE::DIRECTIVE, INPUT.mark());
  INPUT.eat(1);

  // read name
  while (INPUT && !Exp::BlankOrBreak().Matches(INPUT))
    token.value += INPUT.get();

  // read parameters
  while (true) {
    // first get rid of whitespace
    while (Exp::Blank().Matches(INPUT))
      INPUT.eat(1);

    // break on newline or comment
    if (!INPUT || Exp::Break().Matches(INPUT) || Exp::Comment().Matches(INPUT))
      break;

    // now read parameter
    std::string param;
    while (INPUT && !Exp::BlankOrBreak().Matches(INPUT))
      param += INPUT.get();

    token.params.push_back(param);
  }

  m_tokens.push(token);
}

// DocStart
void Scanner::ScanDocStart() {
  PopAllIndents();
  PopAllSimpleKeys();
  m_simpleKeyAllowed = false;
  m_canBeJSONFlow = false;

  // eat
  Mark mark = INPUT.mark();
  INPUT.eat(3);
  m_tokens.push(Token(Token::TYPE::DOC_START, mark));
}

// DocEnd
void Scanner::ScanDocEnd() {
  PopAllIndents();
  PopAllSimpleKeys();
  m_simpleKeyAllowed = false;
  m_canBeJSONFlow = false;

  // eat
  Mark mark = INPUT.mark();
  INPUT.eat(3);
  m_tokens.push(Token(Token::TYPE::DOC_END, mark));
}

// FlowStart
void Scanner::ScanFlowStart() {
  // flows can be simple keys
  InsertPotentialSimpleKey();
  m_simpleKeyAllowed = true;
  m_canBeJSONFlow = false;

  // eat
  Mark mark = INPUT.mark();
  char ch = INPUT.get();
  FLOW_MARKER flowType = (ch == Keys::FlowSeqStart ? FLOW_MARKER::FLOW_SEQ : FLOW_MARKER::FLOW_MAP);
  m_flows.push(flowType);
  Token::TYPE type =
      (flowType == FLOW_MARKER::FLOW_SEQ ? Token::TYPE::FLOW_SEQ_START : Token::TYPE::FLOW_MAP_START);
  m_tokens.push(Token(type, mark));
}

// FlowEnd
void Scanner::ScanFlowEnd() {
  if (InBlockContext())
    throw ParserException(INPUT.mark(), ErrorMsg::FLOW_END);

  // we might have a solo entry in the flow context
  if (InFlowContext()) {
    if (m_flows.top() == FLOW_MARKER::FLOW_MAP && VerifySimpleKey())
      m_tokens.push(Token(Token::TYPE::VALUE, INPUT.mark()));
    else if (m_flows.top() == FLOW_MARKER::FLOW_SEQ)
      InvalidateSimpleKey();
  }

  m_simpleKeyAllowed = false;
  m_canBeJSONFlow = true;

  // eat
  Mark mark = INPUT.mark();
  char ch = INPUT.get();

  // check that it matches the start
  FLOW_MARKER flowType = (ch == Keys::FlowSeqEnd ? FLOW_MARKER::FLOW_SEQ : FLOW_MARKER::FLOW_MAP);
  if (m_flows.top() != flowType)
    throw ParserException(mark, ErrorMsg::FLOW_END);
  m_flows.pop();

  Token::TYPE type = (flowType == FLOW_MARKER::FLOW_SEQ ? Token::TYPE::FLOW_SEQ_END : Token::TYPE::FLOW_MAP_END);
  m_tokens.push(Token(type, mark));
}

// FlowEntry
void Scanner::ScanFlowEntry() {
  // we might have a solo entry in the flow context
  if (InFlowContext()) {
    if (m_flows.top() == FLOW_MARKER::FLOW_MAP && VerifySimpleKey())
      m_tokens.push(Token(Token::TYPE::VALUE, INPUT.mark()));
    else if (m_flows.top() == FLOW_MARKER::FLOW_SEQ)
      InvalidateSimpleKey();
  }

  m_simpleKeyAllowed = true;
  m_canBeJSONFlow = false;

  // eat
  Mark mark = INPUT.mark();
  INPUT.eat(1);
  m_tokens.push(Token(Token::TYPE::FLOW_ENTRY, mark));
}

// BlockEntry
void Scanner::ScanBlockEntry() {
  // we better be in the block context!
  if (InFlowContext())
    throw ParserException(INPUT.mark(), ErrorMsg::BLOCK_ENTRY);

  // can we put it here?
  if (!m_simpleKeyAllowed)
    throw ParserException(INPUT.mark(), ErrorMsg::BLOCK_ENTRY);

  PushIndentTo(INPUT.column(), IndentMarker::INDENT_TYPE::SEQ);
  m_simpleKeyAllowed = true;
  m_canBeJSONFlow = false;

  // eat
  Mark mark = INPUT.mark();
  INPUT.eat(1);
  m_tokens.push(Token(Token::TYPE::BLOCK_ENTRY, mark));
}

// Key
void Scanner::ScanKey() {
  // handle keys differently in the block context (and manage indents)
  if (InBlockContext()) {
    if (!m_simpleKeyAllowed)
      throw ParserException(INPUT.mark(), ErrorMsg::MAP_KEY);

    PushIndentTo(INPUT.column(), IndentMarker::INDENT_TYPE::MAP);
  }

  // can only put a simple key here if we're in block context
  m_simpleKeyAllowed = InBlockContext();

  // eat
  Mark mark = INPUT.mark();
  INPUT.eat(1);
  m_tokens.push(Token(Token::TYPE::KEY, mark));
}

// Value
void Scanner::ScanValue() {
  // and check that simple key
  bool isSimpleKey = VerifySimpleKey();
  m_canBeJSONFlow = false;

  if (isSimpleKey) {
    // can't follow a simple key with another simple key (dunno why, though - it
    // seems fine)
    m_simpleKeyAllowed = false;
  } else {
    // handle values differently in the block context (and manage indents)
    if (InBlockContext()) {
      if (!m_simpleKeyAllowed)
        throw ParserException(INPUT.mark(), ErrorMsg::MAP_VALUE);

      PushIndentTo(INPUT.column(), IndentMarker::INDENT_TYPE::MAP);
    }

    // can only put a simple key here if we're in block context
    m_simpleKeyAllowed = InBlockContext();
  }

  // eat
  Mark mark = INPUT.mark();
  INPUT.eat(1);
  m_tokens.push(Token(Token::TYPE::VALUE, mark));
}

// AnchorOrAlias
void Scanner::ScanAnchorOrAlias() {
  bool alias;
  std::string name;

  // insert a potential simple key
  InsertPotentialSimpleKey();
  m_simpleKeyAllowed = false;
  m_canBeJSONFlow = false;

  // eat the indicator
  Mark mark = INPUT.mark();
  char indicator = INPUT.get();
  alias = (indicator == Keys::Alias);

  // now eat the content
  while (INPUT && Exp::Anchor().Matches(INPUT))
    name += INPUT.get();

  // we need to have read SOMETHING!
  if (name.empty())
    throw ParserException(INPUT.mark(), alias ? ErrorMsg::ALIAS_NOT_FOUND
                                              : ErrorMsg::ANCHOR_NOT_FOUND);

  // and needs to end correctly
  if (INPUT && !Exp::AnchorEnd().Matches(INPUT))
    throw ParserException(INPUT.mark(), alias ? ErrorMsg::CHAR_IN_ALIAS
                                              : ErrorMsg::CHAR_IN_ANCHOR);

  // and we're done
  Token token(alias ? Token::TYPE::ALIAS : Token::TYPE::ANCHOR, mark);
  token.value = name;
  m_tokens.push(token);
}

// Tag
void Scanner::ScanTag() {
  // insert a potential simple key
  InsertPotentialSimpleKey();
  m_simpleKeyAllowed = false;
  m_canBeJSONFlow = false;

  Token token(Token::TYPE::TAG, INPUT.mark());

  // eat the indicator
  INPUT.get();

  using token_data_t = decltype(token.data);

  if (INPUT && INPUT.peek() == Keys::VerbatimTagStart) {
    std::string tag = ScanVerbatimTag(INPUT);

    token.value = tag;
    token.data = static_cast<token_data_t>(Tag::TYPE::VERBATIM);
  } else {
    bool canBeHandle;
    token.value = ScanTagHandle(INPUT, canBeHandle);
    if (!canBeHandle && token.value.empty())
      token.data = static_cast<token_data_t>(Tag::TYPE::NON_SPECIFIC);
    else if (token.value.empty())
      token.data = static_cast<token_data_t>(Tag::TYPE::SECONDARY_HANDLE);
    else
      token.data = static_cast<token_data_t>(Tag::TYPE::PRIMARY_HANDLE);

    // is there a suffix?
    if (canBeHandle && INPUT.peek() == Keys::Tag) {
      // eat the indicator
      INPUT.get();
      token.params.push_back(ScanTagSuffix(INPUT));
      token.data = static_cast<token_data_t>(Tag::TYPE::NAMED_HANDLE);
    }
  }

  m_tokens.push(token);
}

// PlainScalar
void Scanner::ScanPlainScalar() {
  std::string scalar;

  // set up the scanning parameters
  ScanScalarParams params;
  params.end =
      (InFlowContext() ? &Exp::ScanScalarEndInFlow() : &Exp::ScanScalarEnd());
  params.eatEnd = false;
  params.indent = (InFlowContext() ? 0 : GetTopIndent() + 1);
  params.fold = FOLD::FOLD_FLOW;
  params.eatLeadingWhitespace = true;
  params.trimTrailingSpaces = true;
  params.chomp = CHOMP::STRIP;
  params.onDocIndicator = ACTION::BREAK;
  params.onTabInIndentation = ACTION::THROW;

  // insert a potential simple key
  InsertPotentialSimpleKey();

  Mark mark = INPUT.mark();
  scalar = ScanScalar(INPUT, params);

  // can have a simple key only if we ended the scalar by starting a new line
  m_simpleKeyAllowed = params.leadingSpaces;
  m_canBeJSONFlow = false;

  // finally, check and see if we ended on an illegal character
  // if(Exp::IllegalCharInScalar.Matches(INPUT))
  //	throw ParserException(INPUT.mark(), ErrorMsg::CHAR_IN_SCALAR);

  Token token(Token::TYPE::PLAIN_SCALAR, mark);
  token.value = scalar;
  m_tokens.push(token);
}

// QuotedScalar
void Scanner::ScanQuotedScalar() {
  std::string scalar;

  // peek at single or double quote (don't eat because we need to preserve (for
  // the time being) the input position)
  char quote = INPUT.peek();
  bool single = (quote == '\'');

  // setup the scanning parameters
  ScanScalarParams params;
  RegEx end = (single ? RegEx(quote) & !Exp::EscSingleQuote() : RegEx(quote));
  params.end = &end;
  params.eatEnd = true;
  params.escape = (single ? '\'' : '\\');
  params.indent = 0;
  params.fold = FOLD::FOLD_FLOW;
  params.eatLeadingWhitespace = true;
  params.trimTrailingSpaces = false;
  params.chomp = CHOMP::CLIP;
  params.onDocIndicator = ACTION::THROW;

  // insert a potential simple key
  InsertPotentialSimpleKey();

  Mark mark = INPUT.mark();

  // now eat that opening quote
  INPUT.get();

  // and scan
  scalar = ScanScalar(INPUT, params);
  m_simpleKeyAllowed = false;
  m_canBeJSONFlow = true;

  Token token(Token::TYPE::NON_PLAIN_SCALAR, mark);
  token.value = scalar;
  m_tokens.push(token);
}

// BlockScalarToken
// . These need a little extra processing beforehand.
// . We need to scan the line where the indicator is (this doesn't count as part
// of the scalar),
//   and then we need to figure out what level of indentation we'll be using.
void Scanner::ScanBlockScalar() {
  std::string scalar;

  ScanScalarParams params;
  params.indent = 1;
  params.detectIndent = true;

  // eat block indicator ('|' or '>')
  Mark mark = INPUT.mark();
  char indicator = INPUT.get();
  params.fold = (indicator == Keys::FoldedScalar ? FOLD::FOLD_BLOCK : FOLD::DONT_FOLD);

  // eat chomping/indentation indicators
  params.chomp = CHOMP::CLIP;
  int n = Exp::Chomp().Match(INPUT);
  for (int i = 0; i < n; i++) {
    char ch = INPUT.get();
    if (ch == '+')
      params.chomp = CHOMP::KEEP;
    else if (ch == '-')
      params.chomp = CHOMP::STRIP;
    else if (Exp::Digit().Matches(ch)) {
      if (ch == '0')
        throw ParserException(INPUT.mark(), ErrorMsg::ZERO_INDENT_IN_BLOCK);

      params.indent = ch - '0';
      params.detectIndent = false;
    }
  }

  // now eat whitespace
  while (Exp::Blank().Matches(INPUT))
    INPUT.eat(1);

  // and comments to the end of the line
  if (Exp::Comment().Matches(INPUT))
    while (INPUT && !Exp::Break().Matches(INPUT))
      INPUT.eat(1);

  // if it's not a line break, then we ran into a bad character inline
  if (INPUT && !Exp::Break().Matches(INPUT))
    throw ParserException(INPUT.mark(), ErrorMsg::CHAR_IN_BLOCK);

  // set the initial indentation
  if (GetTopIndent() >= 0)
    params.indent += GetTopIndent();

  params.eatLeadingWhitespace = false;
  params.trimTrailingSpaces = false;
  params.onTabInIndentation = ACTION::THROW;

  scalar = ScanScalar(INPUT, params);

  // simple keys always ok after block scalars (since we're gonna start a new
  // line anyways)
  m_simpleKeyAllowed = true;
  m_canBeJSONFlow = false;

  Token token(Token::TYPE::NON_PLAIN_SCALAR, mark);
  token.value = scalar;
  m_tokens.push(token);
}
}  // namespace YAML

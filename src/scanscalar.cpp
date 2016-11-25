#include "scanscalar.h"

#include <algorithm>

#include "exp.h"
#include "stream.h"
#include "yaml-cpp/exceptions.h"  // IWYU pragma: keep

namespace YAML {

int ScanScalar::MatchScalarEmpty(const Stream&) {
  // This is checked by !INPUT as well
  return -1;
}

int ScanScalar::MatchScalarSingleQuoted(const Stream& in) {
  using namespace Exp;
  return (Matcher<Char<'\''>>::Matches(in) &&
          !EscSingleQuote::Matches(in)) ? 1 : -1;
}

int ScanScalar::MatchScalarDoubleQuoted(const Stream& in) {
  using namespace Exp;
  return Matcher<Char<'\"'>>::Match(in);
}

int ScanScalar::MatchScalarEnd(const Stream& in) {
  using namespace Exp;
  using ScalarEnd = Matcher<
      OR < SEQ < Char<':'>,
                 OR < detail::BlankOrBreak,
                      Empty >>,
           SEQ < detail::BlankOrBreak,
                 detail::Comment>>>;

  return ScalarEnd::Match(in);
}

int ScanScalar::MatchScalarEndInFlow(const Stream& in) {
  using namespace Exp;
  using ScalarEndInFlow = Matcher <
      OR < SEQ < Char<':'>,
                 OR < detail::BlankOrBreak,
                      Char<','>,
                      Char<']'>,
                      Char<'}'>,
                      Empty >>,
           Char<','>,
           Char<'?'>,
           Char<'['>,
           Char<']'>,
           Char<'{'>,
           Char<'}'>,
           SEQ < detail::BlankOrBreak,
                 detail::Comment>>>;

  return ScalarEndInFlow::Match(in);
}

bool ScanScalar::MatchDocIndicator(const Stream& in) {
 using namespace Exp;
 using DocIndicator = Matcher<OR <detail::DocStart, detail::DocEnd>>;

 return DocIndicator::Matches(in);
}

bool ScanScalar::CheckDocIndicator(Stream& INPUT, ScanScalarParams& params) {
  if (MatchDocIndicator(INPUT)) {
    if (params.onDocIndicator == BREAK) {
      return true;
    } else if (params.onDocIndicator == THROW) {
      throw ParserException(INPUT.mark(), ErrorMsg::DOC_IN_SCALAR);
    }
  }
  return false;
}

// ScanScalar
// . This is where the scalar magic happens.
//
// . We do the scanning in three phases:
//   1. Scan until newline
//   2. Eat newline
//   3. Scan leading blanks.
//
// . Depending on the parameters given, we store or stop
//   and different places in the above flow.
std::string ScanScalar::Apply(Stream& INPUT, ScanScalarParams& params) {
  bool foundNonEmptyLine = false;
  bool pastOpeningBreak = (params.fold == FOLD_FLOW);
  bool emptyLine = false, moreIndented = false;
  int foldedNewlineCount = 0;
  bool foldedNewlineStartedMoreIndented = false;
  std::size_t lastEscapedChar = std::string::npos;
  std::string scalar;
  params.leadingSpaces = false;

  while (INPUT) {
    // ********************************
    // Phase #1: scan until line ending

    bool escapedNewline = false;
    std::size_t lastNonWhitespaceChar = scalar.size();

    while (1) {

      // find end posiion
      if (params.end(INPUT) >= 0) {
        break;
      }

      if (!INPUT) {
        break;
      }

      // find break posiion
      char ch = INPUT.peek();

      bool isWhiteSpace = (ch == ' ' || ch == '\t');

      if (!isWhiteSpace) {
          if (ch == '\n' || (ch == '\r' && Exp::Break::Matches(INPUT))) {
              break;
          }
          // document indicator?
          if (INPUT.column() == 0 && CheckDocIndicator(INPUT, params)) {
              break;
          }
      }

      foundNonEmptyLine = true;
      pastOpeningBreak = true;

      if (params.escape != ch) {
        // just add the character
        scalar += ch;
        INPUT.eat();

        if (!isWhiteSpace) {
          lastNonWhitespaceChar = scalar.size();
        }

      } else {
        // escaped newline? (only if we're escaping on slash)
        if (params.escape == '\\' && Exp::EscBreak::Matches(INPUT)) {
          // eat escape character and get out (but preserve trailing whitespace!)
          INPUT.eat();
          lastNonWhitespaceChar = scalar.size();
          lastEscapedChar = scalar.size();
          escapedNewline = true;
          break;

        } else {
          scalar += Exp::Escape(INPUT);
          lastNonWhitespaceChar = scalar.size();
          lastEscapedChar = scalar.size();
        }
      }
    } // end while(1)

    // eof? if we're looking to eat something, then we throw
    if (!INPUT) {
      if (params.eatEnd) {
        throw ParserException(INPUT.mark(), ErrorMsg::EOF_IN_SCALAR);
      }
      break;
    }

    // doc indicator?
    if (params.onDocIndicator == BREAK &&
        INPUT.column() == 0 &&
        MatchDocIndicator(INPUT)) {
      break;
    }

    // are we done via character match?
    if (int n = params.end(INPUT) >= 0) {
      if (params.eatEnd) {
        INPUT.eat(n);
      }
      break;
    }

    // do we remove trailing whitespace?
    if (params.fold == FOLD_FLOW)
      scalar.erase(lastNonWhitespaceChar);

    // ********************************
    // Phase #2: eat line ending
    if (int n = Exp::Break::Match(INPUT)) {
        INPUT.eat(n);
    }
    // ********************************
    // Phase #3: scan initial spaces

    // first the required indentation
    while (INPUT.peek() == ' ' &&
           (INPUT.column() < params.indent ||
            (params.detectIndent && !foundNonEmptyLine)) &&
           !(params.end(INPUT) >= 0)) {
      INPUT.eat(1);
    }

    // update indent if we're auto-detecting
    if (params.detectIndent && !foundNonEmptyLine) {
      params.indent = std::max(params.indent, INPUT.column());
    }

    // and then the rest of the whitespace
    for (char c = INPUT.peek(); (c == ' ' || c == '\t'); c = INPUT.peek()) {
      // we check for tabs that masquerade as indentation
      if (c == '\t' && INPUT.column() < params.indent &&
          params.onTabInIndentation == THROW) {
        throw ParserException(INPUT.mark(), ErrorMsg::TAB_IN_INDENTATION);
      }

      if (!params.eatLeadingWhitespace) {
        break;
      }

      if (params.end(INPUT) >= 0) {
        break;
      }

      INPUT.eat(1);
    }

    // was this an empty line?
    bool nextEmptyLine = Exp::Break::Matches(INPUT);
    bool nextMoreIndented = Exp::Blank::Matches(INPUT);
    if (params.fold == FOLD_BLOCK && foldedNewlineCount == 0 && nextEmptyLine)
      foldedNewlineStartedMoreIndented = moreIndented;

    // for block scalars, we always start with a newline, so we should ignore it
    // (not fold or keep)
    if (pastOpeningBreak) {
      switch (params.fold) {
        case DONT_FOLD:
          scalar += "\n";
          break;
        case FOLD_BLOCK:
          if (!emptyLine && !nextEmptyLine && !moreIndented &&
              !nextMoreIndented && INPUT.column() >= params.indent) {
            scalar += " ";
          } else if (nextEmptyLine) {
            foldedNewlineCount++;
          } else {
            scalar += "\n";
          }

          if (!nextEmptyLine && foldedNewlineCount > 0) {
            scalar += std::string(foldedNewlineCount - 1, '\n');
            if (foldedNewlineStartedMoreIndented ||
                nextMoreIndented | !foundNonEmptyLine) {
              scalar += "\n";
            }
            foldedNewlineCount = 0;
          }
          break;
        case FOLD_FLOW:
          if (nextEmptyLine) {
            scalar += "\n";
          } else if (!emptyLine && !nextEmptyLine && !escapedNewline) {
            scalar += " ";
          }
          break;
      }
    }

    emptyLine = nextEmptyLine;
    moreIndented = nextMoreIndented;
    pastOpeningBreak = true;

    // are we done via indentation?
    if (!emptyLine && INPUT.column() < params.indent) {
      params.leadingSpaces = true;
      break;
    }
  }

  // post-processing
  if (params.trimTrailingSpaces) {
    std::size_t pos = scalar.find_last_not_of(' ');
    if (lastEscapedChar != std::string::npos) {
      if (pos < lastEscapedChar || pos == std::string::npos) {
        pos = lastEscapedChar;
      }
    }
    if (pos < scalar.size()) {
      scalar.erase(pos + 1);
    }
  }

  switch (params.chomp) {
    case CLIP: {
      std::size_t pos = scalar.find_last_not_of('\n');
      if (lastEscapedChar != std::string::npos) {
        if (pos < lastEscapedChar || pos == std::string::npos) {
          pos = lastEscapedChar;
        }
      }
      if (pos == std::string::npos) {
        scalar.erase();
      } else if (pos + 1 < scalar.size()) {
        scalar.erase(pos + 2);
      }
    } break;
    case STRIP: {
      std::size_t pos = scalar.find_last_not_of('\n');
      if (lastEscapedChar != std::string::npos) {
        if (pos < lastEscapedChar || pos == std::string::npos) {
          pos = lastEscapedChar;
        }
      }
      if (pos == std::string::npos) {
        scalar.erase();
      } else if (pos < scalar.size()) {
        scalar.erase(pos + 1);
      }
    } break;
    default:
      break;
  }

  return scalar;
}
}

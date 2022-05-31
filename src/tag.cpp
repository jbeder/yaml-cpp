#include <cassert>
#include <stdexcept>

#include "directives.h"  // IWYU pragma: keep
#include "tag.h"
#include "token.h"

namespace YAML {
Tag::Tag(const Token& token)
    : type(static_cast<TYPE>(token.data)), handle{}, value{} {
  switch (type) {
    case VERBATIM:
      value = token.value;
      break;
    case PRIMARY_HANDLE:
      value = token.value;
      break;
    case SECONDARY_HANDLE:
      value = token.value;
      break;
    case NAMED_HANDLE:
      handle = token.value;
      value = token.params[0];
      break;
    case NON_SPECIFIC:
      break;
    default:
      assert(false);
  }
}

std::string Tag::Translate(const Directives& directives) {
  std::string result;

  switch (type) {
    case VERBATIM:
      result = value;
      break;
    case PRIMARY_HANDLE:
      result = directives.TranslateTagHandle("!") + value;
      break;
    case SECONDARY_HANDLE:
      result = directives.TranslateTagHandle("!!") + value;
      break;
    case NAMED_HANDLE:
      result = directives.TranslateTagHandle("!" + handle + "!") + value;
      break;
    case NON_SPECIFIC:
      // TODO:
      result = "!";
      break;
    default:
      assert(false);
      throw std::runtime_error("yaml-cpp: internal error, bad tag type");
  }

  return result;
}
}  // namespace YAML

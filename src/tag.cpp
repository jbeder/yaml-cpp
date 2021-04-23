#include <cassert>
#include <stdexcept>

#include "directives.h"  // IWYU pragma: keep
#include "tag.h"
#include "token.h"

namespace YAML {
Tag::Tag(const Token& token)
    : type(static_cast<TYPE>(token.data)), handle{}, value{} {
  switch (type) {
    case TYPE::VERBATIM:
      value = token.value;
      break;
    case TYPE::PRIMARY_HANDLE:
      value = token.value;
      break;
    case TYPE::SECONDARY_HANDLE:
      value = token.value;
      break;
    case TYPE::NAMED_HANDLE:
      handle = token.value;
      value = token.params[0];
      break;
    case TYPE::NON_SPECIFIC:
      break;
    default:
      assert(false);
  }
}

const std::string Tag::Translate(const Directives& directives) {
  switch (type) {
    case TYPE::VERBATIM:
      return value;
    case TYPE::PRIMARY_HANDLE:
      return directives.TranslateTagHandle("!") + value;
    case TYPE::SECONDARY_HANDLE:
      return directives.TranslateTagHandle("!!") + value;
    case TYPE::NAMED_HANDLE:
      return directives.TranslateTagHandle("!" + handle + "!") + value;
    case TYPE::NON_SPECIFIC:
      // TODO:
      return "!";
    default:
      assert(false);
  }
  throw std::runtime_error("yaml-cpp: internal error, bad tag type");
}
}  // namespace YAML

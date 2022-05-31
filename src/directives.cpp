#include "directives.h"

namespace YAML {
Directives::Directives() : version{true, 1, 2}, tags{} {}

std::string Directives::TranslateTagHandle(
    const std::string& handle) const {
  std::string result;
  auto it = tags.find(handle);
  if (it == tags.end()) {
    if (handle == "!!")
      result = "tag:yaml.org,2002:";
    else
      result = handle;
  } else {
    result = it->second;
  }

  return result;
}
}  // namespace YAML

#include "yaml-cpp/null.h"
#include <cstring>

namespace YAML {
_Null Null;

template <std::size_t N>
static bool same(const char* str, std::size_t size, const char (&literal)[N]) {
  constexpr int literalSize = N - 1; // minus null terminator
  return size == literalSize && std::strncmp(str, literal, literalSize) == 0;
}

bool IsNullString(const char* str, std::size_t size) {
  return size == 0 || same(str, size, "~") || same(str, size, "null") ||
         same(str, size, "Null") || same(str, size, "NULL");
}
}  // namespace YAML

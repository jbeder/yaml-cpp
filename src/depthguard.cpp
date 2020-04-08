#include "yaml-cpp/depthguard.h"

namespace YAML {

DeepRecursion::DeepRecursion(int at_depth, const Mark& mark_, const std::string& msg_)
    : ParserException(mark_, msg_),
      m_atDepth(at_depth) {
}

int DeepRecursion::AtDepth() const {
  return m_atDepth;
}

} // namespace YAML

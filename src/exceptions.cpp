#include "yaml-cpp/exceptions.h"

// This is here for compatibility with older versions of Visual Studio
// which don't support noexcept
#if defined(_MSC_VER) && _MSC_VER < 1900
    #define YAML_CPP_NOEXCEPT _NOEXCEPT
#else
    #define YAML_CPP_NOEXCEPT noexcept
#endif

namespace YAML {

// These destructors are defined out-of-line so the vtable is only emitted once.
Exception::~Exception() YAML_CPP_NOEXCEPT = default;
ParserException::~ParserException() YAML_CPP_NOEXCEPT = default;
RepresentationException::~RepresentationException() YAML_CPP_NOEXCEPT = default;
InvalidScalar::~InvalidScalar() YAML_CPP_NOEXCEPT = default;
KeyNotFound::~KeyNotFound() YAML_CPP_NOEXCEPT = default;
InvalidNode::~InvalidNode() YAML_CPP_NOEXCEPT = default;
BadConversion::~BadConversion() YAML_CPP_NOEXCEPT = default;
BadDereference::~BadDereference() YAML_CPP_NOEXCEPT = default;
BadSubscript::~BadSubscript() YAML_CPP_NOEXCEPT = default;
BadPushback::~BadPushback() YAML_CPP_NOEXCEPT = default;
BadInsert::~BadInsert() YAML_CPP_NOEXCEPT = default;
EmitterException::~EmitterException() YAML_CPP_NOEXCEPT = default;
BadFile::~BadFile() YAML_CPP_NOEXCEPT = default;
}

#undef YAML_CPP_NOEXCEPT



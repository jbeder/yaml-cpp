#include "yaml-cpp/exceptions.h"
#include "yaml-cpp/noexcept.h"

namespace YAML {

namespace {
  thread_local ExceptionHandle handle_exception_local = nullptr;
  ExceptionHandle handle_exception = nullptr;
}
YAML_CPP_API void set_handle_exception_local(ExceptionHandle handle) {
  handle_exception_local = handle;
}

YAML_CPP_API void set_handle_exception(ExceptionHandle handle) {
  handle_exception = handle;
}

YAML_CPP_API ExceptionHandle get_handle_exception_local() {
  return handle_exception_local;
}
YAML_CPP_API ExceptionHandle get_handle_exception() {
  return handle_exception;
}


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
NonUniqueMapKey::~NonUniqueMapKey() YAML_CPP_NOEXCEPT = default;
}  // namespace YAML

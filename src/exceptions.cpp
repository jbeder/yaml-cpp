#include "yaml-cpp/exceptions.h"

namespace YAML {

// These destructors are defined out-of-line so the vtable is only emitted once.
Exception::~Exception() NOEXCEPT {}
ParserException::~ParserException() NOEXCEPT {}
RepresentationException::~RepresentationException() NOEXCEPT {}
InvalidScalar::~InvalidScalar() NOEXCEPT {}
KeyNotFound::~KeyNotFound() NOEXCEPT {}
InvalidNode::~InvalidNode() NOEXCEPT {}
BadConversion::~BadConversion() NOEXCEPT {}
BadDereference::~BadDereference() NOEXCEPT {}
BadSubscript::~BadSubscript() NOEXCEPT {}
BadPushback::~BadPushback() NOEXCEPT {}
BadInsert::~BadInsert() NOEXCEPT {}
EmitterException::~EmitterException() NOEXCEPT {}
BadFile::~BadFile() NOEXCEPT {}
}

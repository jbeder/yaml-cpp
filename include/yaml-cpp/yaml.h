#ifndef YAML_H_62B23520_7C8E_11DE_8A39_0800200C9A66
#define YAML_H_62B23520_7C8E_11DE_8A39_0800200C9A66

#if defined(_MSC_VER) ||                                            \
    (defined(__GNUC__) && (__GNUC__ == 3 && __GNUC_MINOR__ >= 4) || \
     (__GNUC__ >= 4))  // GCC supports "pragma once" correctly since 3.4
#pragma once
#endif

// IWYU pragma: begin_exports

#include "yaml-cpp/parser.h"  // IWYU pragma: export
#include "yaml-cpp/emitter.h"  // IWYU pragma: export
#include "yaml-cpp/emitterstyle.h"  // IWYU pragma: export
#include "yaml-cpp/stlemitter.h"  // IWYU pragma: export
#include "yaml-cpp/exceptions.h"  // IWYU pragma: export

#include "yaml-cpp/node/node.h"  // IWYU pragma: export
#include "yaml-cpp/node/impl.h"  // IWYU pragma: export
#include "yaml-cpp/node/convert.h"  // IWYU pragma: export
#include "yaml-cpp/node/iterator.h"  // IWYU pragma: export
#include "yaml-cpp/node/detail/impl.h"  // IWYU pragma: export
#include "yaml-cpp/node/parse.h"  // IWYU pragma: export
#include "yaml-cpp/node/emit.h"  // IWYU pragma: export

// IWYU pragma: end_exports

#endif  // YAML_H_62B23520_7C8E_11DE_8A39_0800200C9A66

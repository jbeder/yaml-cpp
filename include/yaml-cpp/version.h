#ifndef YAML_CPP_VERSION_H
#define YAML_CPP_VERSION_H

#if defined(_MSC_VER) ||                                            \
    (defined(__GNUC__) && (__GNUC__ == 3 && __GNUC_MINOR__ >= 4) || \
     (__GNUC__ >= 4))  // GCC supports "pragma once" correctly since 3.4
#pragma once
#endif

#define YAML_CPP_VERSION_MAJOR 0
#define YAML_CPP_VERSION_MINOR 5
#define YAML_CPP_VERSION_PATCH 2

// String representation of the current version (ie. "0.1.2")
#define YAML_CPP_VERSION "0.5.2"

#endif  // YAML_CPP_VERSION_H

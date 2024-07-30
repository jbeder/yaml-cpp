// SPDX-FileCopyrightText: 2024 Simon Gene Gottlieb
// SPDX-License-Identifier: MIT

#ifndef YAML_H_FP_TO_STRING
#define YAML_H_FP_TO_STRING

#include "yaml-cpp/dll.h"

#include <string>

namespace YAML {
// "precision = 0" refers to shortest known unique representation of the value
YAML_CPP_API std::string FpToString(float v, size_t precision = 0);
YAML_CPP_API std::string FpToString(double v, size_t precision = 0);
YAML_CPP_API std::string FpToString(long double v, size_t precision = 0);
}

#endif

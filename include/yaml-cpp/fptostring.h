// SPDX-FileCopyrightText: 2024 Simon Gene Gottlieb
// SPDX-License-Identifier: MIT

#ifndef YAML_H_FP_TO_STRING
#define YAML_H_FP_TO_STRING

#include <string>

namespace YAML {
// "precision = 0" refers to shortest known unique representation of the value
std::string FpToString(float v, size_t precision = 0);
std::string FpToString(double v, size_t precision = 0);
std::string FpToString(long double v, size_t precision = 0);
}

#endif

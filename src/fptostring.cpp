#include "yaml-cpp/fptostring.h"
#include "contrib/dragonbox.h"

#include <array>
#include <cassert>
#include <cmath>
#include <limits>
#include <sstream>
#include <tuple>

namespace YAML {
namespace detail {
namespace fp_formatting {

/**
 * Converts a integer into its ASCII digits.
 *
 * @param begin/end - a buffer, must be at least 20bytes long.
 * @param value     - input value.
 * @param width     - minimum number of digits, fill with '0' to the left. Must be equal or smaller than the buffer size.
 * @return          - number of digits filled into the buffer (or -1 if preconditions are not meet)
 *
 * Example:
 * std::array<char, 20> buffer;
 * auto ct = ConvertToChars(buffer.begin(), buffer.end(), 23, 3);
 * assert(ct = 3);
 * assert(buffer[0] == '0');
 * assert(buffer[1] == '2');
 * assert(buffer[2] == '3');
 */
int ConvertToChars(char* begin, char* end, size_t value, int width=1) {
  // precondition of this function (will trigger in debug build)
  assert(width >= 1);
  assert(end >= begin);       // end must be after begin
  assert(end-begin >= width); // Buffer must be large enough
  assert(end-begin >= 20);    // 2^64 has 20digits, so at least 20 digits must be available

  // defensive programming, abort if precondition are not met (will trigger in release build)
  if (width < 1) {
    return -1;
  }
  if (end < begin) {
    return -1;
  }
  if (end-begin < width) {
    return -1;
  }
  if (end-begin < 20) {
    return -1;
  }


  // count number of digits, and fill digits array accordingly
  int digits_ct{};
  while (value > 0) {
    char c = value % 10 + '0';
    value = value / 10;
    digits_ct += 1;
    *(end-digits_ct) = c;
  }
  while(digits_ct < width) {
    assert(digits_ct < 64);
    digits_ct += 1;
    *(end-digits_ct) = '0';
  }
  // move data to the front of the array
  std::memmove(begin, end-digits_ct, digits_ct);
  return digits_ct;
}

/**
 * Converts a float or double to a string.
 *
 * converts a value 'v' to a string. Uses dragonbox for formatting.
 */
template <typename T>
std::string FpToString(T v, int precision = 0) {
  // hard coded constant, at which exponent should switch to a scientific notation
  int const lowerExponentThreshold = -5;
  int const upperExponentThreshold =  (precision==0)?6:precision;
  if (precision == 0) {
    precision = 6;
  }

  // dragonbox/to_decimal does not handle value 0, inf, NaN
  if (v == 0 || std::isinf(v) || std::isnan(v)) {
    std::stringstream ss;
    ss.imbue(std::locale("C"));
    ss << v;
    return ss.str();
  }

  auto r = jkj::dragonbox::to_decimal(v);

  auto digits  = std::array<char, 20>{}; // max digits of size_t is 20.
  auto digits_ct = ConvertToChars(digits.data(), digits.data() + digits.size(), r.significand);

  // defensive programming, ConvertToChars arguments are invalid
  if (digits_ct == -1) {
    std::stringstream ss;
    ss.imbue(std::locale("C"));
    ss << v;
    return ss.str();
  }

  // check if requested precision is lower than
  // required digits for exact representation
  if (digits_ct > precision) {
    auto diff = digits_ct - precision;
    r.exponent += diff;
    digits_ct = precision;

    // round numbers if required
    if (digits[digits_ct] >= '5') {
      int i{digits_ct-1};
      digits[i] += 1;
      while (digits[i] == '9'+1) {
        digits_ct -= 1;
        r.exponent += 1;
        if (i > 0) {
          digits[i-1] += 1;
          i -= 1;
        } else {
          digits_ct = 1;
          digits[0] = '1';
          break;
        }
      }
    }
  }

  std::array<char, 28> output_buffer; // max digits of size_t plus sign, a dot and 2 letters for 'e+' or 'e-' and 4 letters for the exponent
  auto output_ptr = &output_buffer[0];

  // print '-' symbol for negative numbers
  if (r.is_negative) {
    *(output_ptr++) = '-';
  }

  // exponent if only a single non-zero digit is before the decimal point
  int const exponent = r.exponent + digits_ct - 1;

  // case 1: scientific notation
  if (exponent >= upperExponentThreshold || exponent <= lowerExponentThreshold) {
    // print first digit
    *(output_ptr++) = digits[0];

    // print digits after decimal point
    if (digits_ct > 1) {
      *(output_ptr++) = '.';
      // print significant numbers after decimal point
      for (int i{1}; i < digits_ct; ++i) {
         *(output_ptr++) = digits[i];
      }
    }
    *(output_ptr++) = 'e';
    *(output_ptr++) = (exponent>=0)?'+':'-';
    auto exp_digits = std::array<char, 20>{};
    auto exp_digits_ct = ConvertToChars(exp_digits.data(), exp_digits.data() + exp_digits.size(), std::abs(exponent), /*.precision=*/ 2);

    // defensive programming, ConvertToChars arguments are invalid
    if (exp_digits_ct == -1) {
      std::stringstream ss;
      ss.imbue(std::locale("C"));
      ss << v;
      return ss.str();
    }

    for (int i{0}; i < exp_digits_ct; ++i) {
      *(output_ptr++) = exp_digits[i];
    }

  // case 2: default notation
  } else {
    auto const digits_end   = digits.begin() + digits_ct;
    auto digits_iter    = digits.begin();

    // print digits before point
    int const before_decimal_digits = digits_ct + r.exponent;
    if (before_decimal_digits > 0) {
      // print digits before point
      for (int i{0}; i < std::min(before_decimal_digits, digits_ct); ++i) {
        *(output_ptr++) = *(digits_iter++);
      }
      // print trailing zeros before point
      for (int i{0}; i < before_decimal_digits - digits_ct; ++i) {
        *(output_ptr++) = '0';
      }

    // print 0 before point if none where printed before
    } else {
      *(output_ptr++) = '0';
    }

    if (digits_iter != digits_end) {
      *(output_ptr++) = '.';
      // print 0 after decimal point, to fill until first digits
      int const after_decimal_zeros = -digits_ct - r.exponent;
      for (int i{0}; i < after_decimal_zeros; ++i) {
         *(output_ptr++) = '0';
      }

      // print significant numbers after decimal point
      for (;digits_iter < digits_end; ++digits_iter) {
         *(output_ptr++) = *digits_iter;
      }
    }
  }
  *output_ptr = '\0';
  return std::string{&output_buffer[0], output_ptr};
}

}
}

std::string FpToString(float v, size_t precision) {
  return detail::fp_formatting::FpToString(v, precision);
}

std::string FpToString(double v, size_t precision) {
  return detail::fp_formatting::FpToString(v, precision);
}

/**
 * dragonbox only works for floats/doubles not long double
 */
std::string FpToString(long double v, size_t precision) {
  std::stringstream ss;
  ss.imbue(std::locale("C"));
  if (precision == 0) {
     precision = std::numeric_limits<long double>::max_digits10;
  }
  ss.precision(precision);
  ss << v;
  return ss.str();
}

}

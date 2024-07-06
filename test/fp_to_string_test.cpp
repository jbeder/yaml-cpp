#include "yaml-cpp/fp_to_string.h"
#include "gtest/gtest.h"

namespace YAML {
namespace {

/**
 * Helper function, that converts double to string as std::stringstream would do
 */
template <typename T>
static std::string convert_with_stringstream(T v, size_t precision = 0) {
    std::stringstream ss;
    if (precision > 0) {
        ss << std::setprecision(precision);
    }
    ss << v;
    return ss.str();
}

// Caution: Test involving 'convert_with_stringstream' are based on std::stringstream
// having certain printing behavior, if these changes, the unit test might fail.
// This is not a fault of fp_to_string just a weakness of the way these
// tests are constructed

TEST(FpToStringTest, conversion_double) {
  // Issue motivating fp_to_string function,
  // https://github.com/jbeder/yaml-cpp/issues/1289
  // Original problem at hand:
  EXPECT_EQ("34.34", fp_to_string(34.34));
  EXPECT_EQ("56.56", fp_to_string(56.56));
  EXPECT_EQ("12.12", fp_to_string(12.12));
  EXPECT_EQ("78.78", fp_to_string(78.78));

  // Special challenge with rounding
  // https://github.com/jbeder/yaml-cpp/issues/1289#issuecomment-2211705536
  EXPECT_EQ("1.54743e+26", fp_to_string(1.5474250491e+26f));
  EXPECT_EQ(convert_with_stringstream(1.5474250491e+26f), fp_to_string(1.5474250491e+26f));
  EXPECT_EQ("1.5474251e+26", fp_to_string(1.5474250491e+26f, 8));

  // prints the same way as std::stringstream
  EXPECT_EQ(convert_with_stringstream(1.), fp_to_string(1.));
  EXPECT_EQ(convert_with_stringstream(1e0), fp_to_string(1e0));
  EXPECT_EQ(convert_with_stringstream(1e1), fp_to_string(1e1));
  EXPECT_EQ(convert_with_stringstream(1e2), fp_to_string(1e2));
  EXPECT_EQ(convert_with_stringstream(1e3), fp_to_string(1e3));
  EXPECT_EQ(convert_with_stringstream(1e4), fp_to_string(1e4));
  EXPECT_EQ(convert_with_stringstream(1e5), fp_to_string(1e5));
  EXPECT_EQ(convert_with_stringstream(1e6), fp_to_string(1e6));
  EXPECT_EQ(convert_with_stringstream(1e7), fp_to_string(1e7));
  EXPECT_EQ(convert_with_stringstream(1e8), fp_to_string(1e8));
  EXPECT_EQ(convert_with_stringstream(1e9), fp_to_string(1e9));

  // Print by default values below 1e6 without scientific notation
  EXPECT_EQ("1", fp_to_string(1.));
  EXPECT_EQ("1", fp_to_string(1e0));
  EXPECT_EQ("10", fp_to_string(1e1));
  EXPECT_EQ("100", fp_to_string(1e2));
  EXPECT_EQ("1000", fp_to_string(1e3));
  EXPECT_EQ("10000", fp_to_string(1e4));
  EXPECT_EQ("100000", fp_to_string(1e5));
  EXPECT_EQ("1e+06", fp_to_string(1e6));
  EXPECT_EQ("1e+07", fp_to_string(1e7));
  EXPECT_EQ("1e+08", fp_to_string(1e8));
  EXPECT_EQ("1e+09", fp_to_string(1e9));

  // prints the same way as std::stringstream
  EXPECT_EQ(convert_with_stringstream(1.), fp_to_string(1.));
  EXPECT_EQ(convert_with_stringstream(1e-0), fp_to_string(1e-0));
  EXPECT_EQ(convert_with_stringstream(1e-1), fp_to_string(1e-1));
  EXPECT_EQ(convert_with_stringstream(1e-2), fp_to_string(1e-2));
  EXPECT_EQ(convert_with_stringstream(1e-3), fp_to_string(1e-3));
  EXPECT_EQ(convert_with_stringstream(1e-4), fp_to_string(1e-4));
  EXPECT_EQ(convert_with_stringstream(1e-5), fp_to_string(1e-5));
  EXPECT_EQ(convert_with_stringstream(1e-6), fp_to_string(1e-6));
  EXPECT_EQ(convert_with_stringstream(1e-7), fp_to_string(1e-7));
  EXPECT_EQ(convert_with_stringstream(1e-8), fp_to_string(1e-8));
  EXPECT_EQ(convert_with_stringstream(1e-9), fp_to_string(1e-9));

  // Print by default values above 1e-5 without scientific notation
  EXPECT_EQ("1", fp_to_string(1.));
  EXPECT_EQ("1", fp_to_string(1e-0));
  EXPECT_EQ("0.1", fp_to_string(1e-1));
  EXPECT_EQ("0.01", fp_to_string(1e-2));
  EXPECT_EQ("0.001", fp_to_string(1e-3));
  EXPECT_EQ("0.0001", fp_to_string(1e-4));
  EXPECT_EQ("1e-05", fp_to_string(1e-5));
  EXPECT_EQ("1e-06", fp_to_string(1e-6));
  EXPECT_EQ("1e-07", fp_to_string(1e-7));
  EXPECT_EQ("1e-08", fp_to_string(1e-8));
  EXPECT_EQ("1e-09", fp_to_string(1e-9));

  // changing precision has the same effect as std::stringstream
  EXPECT_EQ(convert_with_stringstream(123., 1), fp_to_string(123., 1));
  EXPECT_EQ(convert_with_stringstream(1234567., 7), fp_to_string(1234567., 7));
  EXPECT_EQ(convert_with_stringstream(12345.67, 7), fp_to_string(12345.67, 7));
  EXPECT_EQ(convert_with_stringstream(1234567e-9, 7), fp_to_string(1234567e-9, 7));
  EXPECT_EQ(convert_with_stringstream(1234567e-9, 1), fp_to_string(1234567e-9, 1));

 // known example that is difficult to round
  EXPECT_EQ("1", fp_to_string(0.9999, 2));
  EXPECT_EQ("-1", fp_to_string(-0.9999, 2));

  // some more random tests
  EXPECT_EQ("1.25", fp_to_string(1.25));
  EXPECT_EQ("34.34", fp_to_string(34.34));
  EXPECT_EQ("1e+20", fp_to_string(1e+20));
  EXPECT_EQ("1.1e+20", fp_to_string(1.1e+20));
  EXPECT_EQ("1e-20", fp_to_string(1e-20));
  EXPECT_EQ("1.1e-20", fp_to_string(1.1e-20));
  EXPECT_EQ("1e-20", fp_to_string(0.1e-19));
  EXPECT_EQ("1.1e-20", fp_to_string(0.11e-19));

  EXPECT_EQ("19", fp_to_string(18.9, 2));
  EXPECT_EQ("20", fp_to_string(19.9, 2));
  EXPECT_EQ("2e+01", fp_to_string(19.9, 1));
  EXPECT_EQ("1.2e-05", fp_to_string(1.234e-5, 2));
  EXPECT_EQ("1.3e-05", fp_to_string(1.299e-5, 2));

  EXPECT_EQ("-1", fp_to_string(-1.));
  EXPECT_EQ("-1.25", fp_to_string(-1.25));
  EXPECT_EQ("-34.34", fp_to_string(-34.34));
  EXPECT_EQ("-1e+20", fp_to_string(-1e+20));
  EXPECT_EQ("-1.1e+20", fp_to_string(-1.1e+20));
  EXPECT_EQ("-1e-20", fp_to_string(-1e-20));
  EXPECT_EQ("-1.1e-20", fp_to_string(-1.1e-20));
  EXPECT_EQ("-1e-20", fp_to_string(-0.1e-19));
  EXPECT_EQ("-1.1e-20", fp_to_string(-0.11e-19));

  EXPECT_EQ("-19", fp_to_string(-18.9, 2));
  EXPECT_EQ("-20", fp_to_string(-19.9, 2));
  EXPECT_EQ("-2e+01", fp_to_string(-19.9, 1));
  EXPECT_EQ("-1.2e-05", fp_to_string(-1.234e-5, 2));
  EXPECT_EQ("-1.3e-05", fp_to_string(-1.299e-5, 2));
}

TEST(FpToStringTest, conversion_float) {
  // Issue motivating fp_to_string function,
  // https://github.com/jbeder/yaml-cpp/issues/1289
  // Original problem at hand:
  EXPECT_EQ("34.34", fp_to_string(34.34f));
  EXPECT_EQ("56.56", fp_to_string(56.56f));
  EXPECT_EQ("12.12", fp_to_string(12.12f));
  EXPECT_EQ("78.78", fp_to_string(78.78f));

  // prints the same way as std::stringstream
  EXPECT_EQ(convert_with_stringstream(1.f), fp_to_string(1.f));
  EXPECT_EQ(convert_with_stringstream(1e0f), fp_to_string(1e0f));
  EXPECT_EQ(convert_with_stringstream(1e1f), fp_to_string(1e1f));
  EXPECT_EQ(convert_with_stringstream(1e2f), fp_to_string(1e2f));
  EXPECT_EQ(convert_with_stringstream(1e3f), fp_to_string(1e3f));
  EXPECT_EQ(convert_with_stringstream(1e4f), fp_to_string(1e4f));
  EXPECT_EQ(convert_with_stringstream(1e5f), fp_to_string(1e5f));
  EXPECT_EQ(convert_with_stringstream(1e6f), fp_to_string(1e6f));
  EXPECT_EQ(convert_with_stringstream(1e7f), fp_to_string(1e7f));
  EXPECT_EQ(convert_with_stringstream(1e8f), fp_to_string(1e8f));
  EXPECT_EQ(convert_with_stringstream(1e9f), fp_to_string(1e9f));

  // Print by default values below 1e6 without scientific notation
  EXPECT_EQ("1", fp_to_string(1.f));
  EXPECT_EQ("1", fp_to_string(1e0f));
  EXPECT_EQ("10", fp_to_string(1e1f));
  EXPECT_EQ("100", fp_to_string(1e2f));
  EXPECT_EQ("1000", fp_to_string(1e3f));
  EXPECT_EQ("10000", fp_to_string(1e4f));
  EXPECT_EQ("100000", fp_to_string(1e5f));
  EXPECT_EQ("1e+06", fp_to_string(1e6f));
  EXPECT_EQ("1e+07", fp_to_string(1e7f));
  EXPECT_EQ("1e+08", fp_to_string(1e8f));
  EXPECT_EQ("1e+09", fp_to_string(1e9f));

  // prints the same way as std::stringstream
  EXPECT_EQ(convert_with_stringstream(1.f), fp_to_string(1.f));
  EXPECT_EQ(convert_with_stringstream(1e-0f), fp_to_string(1e-0f));
  EXPECT_EQ(convert_with_stringstream(1e-1f), fp_to_string(1e-1f));
  EXPECT_EQ(convert_with_stringstream(1e-2f), fp_to_string(1e-2f));
  EXPECT_EQ(convert_with_stringstream(1e-3f), fp_to_string(1e-3f));
  EXPECT_EQ(convert_with_stringstream(1e-4f), fp_to_string(1e-4f));
  EXPECT_EQ(convert_with_stringstream(1e-5f), fp_to_string(1e-5f));
  EXPECT_EQ(convert_with_stringstream(1e-6f), fp_to_string(1e-6f));
  EXPECT_EQ(convert_with_stringstream(1e-7f), fp_to_string(1e-7f));
  EXPECT_EQ(convert_with_stringstream(1e-8f), fp_to_string(1e-8f));
  EXPECT_EQ(convert_with_stringstream(1e-9f), fp_to_string(1e-9f));

  // Print by default values above 1e-5 without scientific notation
  EXPECT_EQ("1", fp_to_string(1.f));
  EXPECT_EQ("1", fp_to_string(1e-0f));
  EXPECT_EQ("0.1", fp_to_string(1e-1f));
  EXPECT_EQ("0.01", fp_to_string(1e-2f));
  EXPECT_EQ("0.001", fp_to_string(1e-3f));
  EXPECT_EQ("0.0001", fp_to_string(1e-4f));
  EXPECT_EQ("1e-05", fp_to_string(1e-5f));
  EXPECT_EQ("1e-06", fp_to_string(1e-6f));
  EXPECT_EQ("1e-07", fp_to_string(1e-7f));
  EXPECT_EQ("1e-08", fp_to_string(1e-8f));
  EXPECT_EQ("1e-09", fp_to_string(1e-9f));

  // changing precision has the same effect as std::stringstream
  EXPECT_EQ(convert_with_stringstream(123.f, 1), fp_to_string(123.f, 1));
  EXPECT_EQ(convert_with_stringstream(1234567.f, 7), fp_to_string(1234567.f, 7));
  EXPECT_EQ(convert_with_stringstream(12345.67f, 7), fp_to_string(12345.67f, 7));
  EXPECT_EQ(convert_with_stringstream(1234567e-9f, 7), fp_to_string(1234567e-9f, 7));
  EXPECT_EQ(convert_with_stringstream(1234567e-9f, 1), fp_to_string(1234567e-9f, 1));

 // known example that is difficult to round
  EXPECT_EQ("1", fp_to_string(0.9999f, 2));
  EXPECT_EQ("-1", fp_to_string(-0.9999f, 2));

  // some more random tests
  EXPECT_EQ("1.25", fp_to_string(1.25f));
  EXPECT_EQ("34.34", fp_to_string(34.34f));
  EXPECT_EQ("1e+20", fp_to_string(1e+20f));
  EXPECT_EQ("1.1e+20", fp_to_string(1.1e+20f));
  EXPECT_EQ("1e-20", fp_to_string(1e-20f));
  EXPECT_EQ("1.1e-20", fp_to_string(1.1e-20f));
  EXPECT_EQ("1e-20", fp_to_string(0.1e-19f));
  EXPECT_EQ("1.1e-20", fp_to_string(0.11e-19f));

  EXPECT_EQ("19", fp_to_string(18.9f, 2));
  EXPECT_EQ("20", fp_to_string(19.9f, 2));
  EXPECT_EQ("2e+01", fp_to_string(19.9f, 1));
  EXPECT_EQ("1.2e-05", fp_to_string(1.234e-5f, 2));
  EXPECT_EQ("1.3e-05", fp_to_string(1.299e-5f, 2));

  EXPECT_EQ("-1", fp_to_string(-1.f));
  EXPECT_EQ("-1.25", fp_to_string(-1.25f));
  EXPECT_EQ("-34.34", fp_to_string(-34.34f));
  EXPECT_EQ("-1e+20", fp_to_string(-1e+20f));
  EXPECT_EQ("-1.1e+20", fp_to_string(-1.1e+20f));
  EXPECT_EQ("-1e-20", fp_to_string(-1e-20f));
  EXPECT_EQ("-1.1e-20", fp_to_string(-1.1e-20f));
  EXPECT_EQ("-1e-20", fp_to_string(-0.1e-19f));
  EXPECT_EQ("-1.1e-20", fp_to_string(-0.11e-19f));

  EXPECT_EQ("-19", fp_to_string(-18.9f, 2));
  EXPECT_EQ("-20", fp_to_string(-19.9f, 2));
  EXPECT_EQ("-2e+01", fp_to_string(-19.9f, 1));
  EXPECT_EQ("-1.2e-05", fp_to_string(-1.234e-5f, 2));
  EXPECT_EQ("-1.3e-05", fp_to_string(-1.299e-5f, 2));
}

}  // namespace
}  // namespace YAML

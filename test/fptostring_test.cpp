#include "yaml-cpp/fptostring.h"
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
// This is not a fault of FpToString just a weakness of the way these
// tests are constructed

TEST(FpToStringTest, conversion_double) {
  // Issue motivating FpToString function,
  // https://github.com/jbeder/yaml-cpp/issues/1289
  // Original problem at hand:
  EXPECT_EQ("34.34", FpToString(34.34));
  EXPECT_EQ("56.56", FpToString(56.56));
  EXPECT_EQ("12.12", FpToString(12.12));
  EXPECT_EQ("78.78", FpToString(78.78));

  // Special challenge with rounding
  // https://github.com/jbeder/yaml-cpp/issues/1289#issuecomment-2211705536
  EXPECT_EQ("1.54743e+26", FpToString(1.5474250491e+26f));
  EXPECT_EQ(convert_with_stringstream(1.5474250491e+26f), FpToString(1.5474250491e+26f));
  EXPECT_EQ("1.5474251e+26", FpToString(1.5474250491e+26f, 8));

  // prints the same way as std::stringstream
  EXPECT_EQ(convert_with_stringstream(1.), FpToString(1.));
  EXPECT_EQ(convert_with_stringstream(1e0), FpToString(1e0));
  EXPECT_EQ(convert_with_stringstream(1e1), FpToString(1e1));
  EXPECT_EQ(convert_with_stringstream(1e2), FpToString(1e2));
  EXPECT_EQ(convert_with_stringstream(1e3), FpToString(1e3));
  EXPECT_EQ(convert_with_stringstream(1e4), FpToString(1e4));
  EXPECT_EQ(convert_with_stringstream(1e5), FpToString(1e5));
  EXPECT_EQ(convert_with_stringstream(1e6), FpToString(1e6));
  EXPECT_EQ(convert_with_stringstream(1e7), FpToString(1e7));
  EXPECT_EQ(convert_with_stringstream(1e8), FpToString(1e8));
  EXPECT_EQ(convert_with_stringstream(1e9), FpToString(1e9));

  // Print by default values below 1e6 without scientific notation
  EXPECT_EQ("1", FpToString(1.));
  EXPECT_EQ("1", FpToString(1e0));
  EXPECT_EQ("10", FpToString(1e1));
  EXPECT_EQ("100", FpToString(1e2));
  EXPECT_EQ("1000", FpToString(1e3));
  EXPECT_EQ("10000", FpToString(1e4));
  EXPECT_EQ("100000", FpToString(1e5));
  EXPECT_EQ("1e+06", FpToString(1e6));
  EXPECT_EQ("1e+07", FpToString(1e7));
  EXPECT_EQ("1e+08", FpToString(1e8));
  EXPECT_EQ("1e+09", FpToString(1e9));

  // prints the same way as std::stringstream
  EXPECT_EQ(convert_with_stringstream(1.), FpToString(1.));
  EXPECT_EQ(convert_with_stringstream(1e-0), FpToString(1e-0));
  EXPECT_EQ(convert_with_stringstream(1e-1), FpToString(1e-1));
  EXPECT_EQ(convert_with_stringstream(1e-2), FpToString(1e-2));
  EXPECT_EQ(convert_with_stringstream(1e-3), FpToString(1e-3));
  EXPECT_EQ(convert_with_stringstream(1e-4), FpToString(1e-4));
  EXPECT_EQ(convert_with_stringstream(1e-5), FpToString(1e-5));
  EXPECT_EQ(convert_with_stringstream(1e-6), FpToString(1e-6));
  EXPECT_EQ(convert_with_stringstream(1e-7), FpToString(1e-7));
  EXPECT_EQ(convert_with_stringstream(1e-8), FpToString(1e-8));
  EXPECT_EQ(convert_with_stringstream(1e-9), FpToString(1e-9));

  // Print by default values above 1e-5 without scientific notation
  EXPECT_EQ("1", FpToString(1.));
  EXPECT_EQ("1", FpToString(1e-0));
  EXPECT_EQ("0.1", FpToString(1e-1));
  EXPECT_EQ("0.01", FpToString(1e-2));
  EXPECT_EQ("0.001", FpToString(1e-3));
  EXPECT_EQ("0.0001", FpToString(1e-4));
  EXPECT_EQ("1e-05", FpToString(1e-5));
  EXPECT_EQ("1e-06", FpToString(1e-6));
  EXPECT_EQ("1e-07", FpToString(1e-7));
  EXPECT_EQ("1e-08", FpToString(1e-8));
  EXPECT_EQ("1e-09", FpToString(1e-9));

  // changing precision has the same effect as std::stringstream
  EXPECT_EQ(convert_with_stringstream(123., 1), FpToString(123., 1));
  EXPECT_EQ(convert_with_stringstream(1234567., 7), FpToString(1234567., 7));
  EXPECT_EQ(convert_with_stringstream(12345.67, 7), FpToString(12345.67, 7));
  EXPECT_EQ(convert_with_stringstream(1234567e-9, 7), FpToString(1234567e-9, 7));
  EXPECT_EQ(convert_with_stringstream(1234567e-9, 1), FpToString(1234567e-9, 1));

 // known example that is difficult to round
  EXPECT_EQ("1", FpToString(0.9999, 2));
  EXPECT_EQ("-1", FpToString(-0.9999, 2));

  // some more random tests
  EXPECT_EQ("1.25", FpToString(1.25));
  EXPECT_EQ("34.34", FpToString(34.34));
  EXPECT_EQ("1e+20", FpToString(1e+20));
  EXPECT_EQ("1.1e+20", FpToString(1.1e+20));
  EXPECT_EQ("1e-20", FpToString(1e-20));
  EXPECT_EQ("1.1e-20", FpToString(1.1e-20));
  EXPECT_EQ("1e-20", FpToString(0.1e-19));
  EXPECT_EQ("1.1e-20", FpToString(0.11e-19));

  EXPECT_EQ("19", FpToString(18.9, 2));
  EXPECT_EQ("20", FpToString(19.9, 2));
  EXPECT_EQ("2e+01", FpToString(19.9, 1));
  EXPECT_EQ("1.2e-05", FpToString(1.234e-5, 2));
  EXPECT_EQ("1.3e-05", FpToString(1.299e-5, 2));

  EXPECT_EQ("-1", FpToString(-1.));
  EXPECT_EQ("-1.25", FpToString(-1.25));
  EXPECT_EQ("-34.34", FpToString(-34.34));
  EXPECT_EQ("-1e+20", FpToString(-1e+20));
  EXPECT_EQ("-1.1e+20", FpToString(-1.1e+20));
  EXPECT_EQ("-1e-20", FpToString(-1e-20));
  EXPECT_EQ("-1.1e-20", FpToString(-1.1e-20));
  EXPECT_EQ("-1e-20", FpToString(-0.1e-19));
  EXPECT_EQ("-1.1e-20", FpToString(-0.11e-19));

  EXPECT_EQ("-19", FpToString(-18.9, 2));
  EXPECT_EQ("-20", FpToString(-19.9, 2));
  EXPECT_EQ("-2e+01", FpToString(-19.9, 1));
  EXPECT_EQ("-1.2e-05", FpToString(-1.234e-5, 2));
  EXPECT_EQ("-1.3e-05", FpToString(-1.299e-5, 2));
}

TEST(FpToStringTest, conversion_float) {
  // Issue motivating FpToString function,
  // https://github.com/jbeder/yaml-cpp/issues/1289
  // Original problem at hand:
  EXPECT_EQ("34.34", FpToString(34.34f));
  EXPECT_EQ("56.56", FpToString(56.56f));
  EXPECT_EQ("12.12", FpToString(12.12f));
  EXPECT_EQ("78.78", FpToString(78.78f));

  // prints the same way as std::stringstream
  EXPECT_EQ(convert_with_stringstream(1.f), FpToString(1.f));
  EXPECT_EQ(convert_with_stringstream(1e0f), FpToString(1e0f));
  EXPECT_EQ(convert_with_stringstream(1e1f), FpToString(1e1f));
  EXPECT_EQ(convert_with_stringstream(1e2f), FpToString(1e2f));
  EXPECT_EQ(convert_with_stringstream(1e3f), FpToString(1e3f));
  EXPECT_EQ(convert_with_stringstream(1e4f), FpToString(1e4f));
  EXPECT_EQ(convert_with_stringstream(1e5f), FpToString(1e5f));
  EXPECT_EQ(convert_with_stringstream(1e6f), FpToString(1e6f));
  EXPECT_EQ(convert_with_stringstream(1e7f), FpToString(1e7f));
  EXPECT_EQ(convert_with_stringstream(1e8f), FpToString(1e8f));
  EXPECT_EQ(convert_with_stringstream(1e9f), FpToString(1e9f));

  // Print by default values below 1e6 without scientific notation
  EXPECT_EQ("1", FpToString(1.f));
  EXPECT_EQ("1", FpToString(1e0f));
  EXPECT_EQ("10", FpToString(1e1f));
  EXPECT_EQ("100", FpToString(1e2f));
  EXPECT_EQ("1000", FpToString(1e3f));
  EXPECT_EQ("10000", FpToString(1e4f));
  EXPECT_EQ("100000", FpToString(1e5f));
  EXPECT_EQ("1e+06", FpToString(1e6f));
  EXPECT_EQ("1e+07", FpToString(1e7f));
  EXPECT_EQ("1e+08", FpToString(1e8f));
  EXPECT_EQ("1e+09", FpToString(1e9f));

  // prints the same way as std::stringstream
  EXPECT_EQ(convert_with_stringstream(1.f), FpToString(1.f));
  EXPECT_EQ(convert_with_stringstream(1e-0f), FpToString(1e-0f));
  EXPECT_EQ(convert_with_stringstream(1e-1f), FpToString(1e-1f));
  EXPECT_EQ(convert_with_stringstream(1e-2f), FpToString(1e-2f));
  EXPECT_EQ(convert_with_stringstream(1e-3f), FpToString(1e-3f));
  EXPECT_EQ(convert_with_stringstream(1e-4f), FpToString(1e-4f));
  EXPECT_EQ(convert_with_stringstream(1e-5f), FpToString(1e-5f));
  EXPECT_EQ(convert_with_stringstream(1e-6f), FpToString(1e-6f));
  EXPECT_EQ(convert_with_stringstream(1e-7f), FpToString(1e-7f));
  EXPECT_EQ(convert_with_stringstream(1e-8f), FpToString(1e-8f));
  EXPECT_EQ(convert_with_stringstream(1e-9f), FpToString(1e-9f));

  // Print by default values above 1e-5 without scientific notation
  EXPECT_EQ("1", FpToString(1.f));
  EXPECT_EQ("1", FpToString(1e-0f));
  EXPECT_EQ("0.1", FpToString(1e-1f));
  EXPECT_EQ("0.01", FpToString(1e-2f));
  EXPECT_EQ("0.001", FpToString(1e-3f));
  EXPECT_EQ("0.0001", FpToString(1e-4f));
  EXPECT_EQ("1e-05", FpToString(1e-5f));
  EXPECT_EQ("1e-06", FpToString(1e-6f));
  EXPECT_EQ("1e-07", FpToString(1e-7f));
  EXPECT_EQ("1e-08", FpToString(1e-8f));
  EXPECT_EQ("1e-09", FpToString(1e-9f));

  // changing precision has the same effect as std::stringstream
  EXPECT_EQ(convert_with_stringstream(123.f, 1), FpToString(123.f, 1));
  EXPECT_EQ(convert_with_stringstream(1234567.f, 7), FpToString(1234567.f, 7));
  EXPECT_EQ(convert_with_stringstream(12345.67f, 7), FpToString(12345.67f, 7));
  EXPECT_EQ(convert_with_stringstream(1234567e-9f, 7), FpToString(1234567e-9f, 7));
  EXPECT_EQ(convert_with_stringstream(1234567e-9f, 1), FpToString(1234567e-9f, 1));

 // known example that is difficult to round
  EXPECT_EQ("1", FpToString(0.9999f, 2));
  EXPECT_EQ("-1", FpToString(-0.9999f, 2));

  // some more random tests
  EXPECT_EQ("1.25", FpToString(1.25f));
  EXPECT_EQ("34.34", FpToString(34.34f));
  EXPECT_EQ("1e+20", FpToString(1e+20f));
  EXPECT_EQ("1.1e+20", FpToString(1.1e+20f));
  EXPECT_EQ("1e-20", FpToString(1e-20f));
  EXPECT_EQ("1.1e-20", FpToString(1.1e-20f));
  EXPECT_EQ("1e-20", FpToString(0.1e-19f));
  EXPECT_EQ("1.1e-20", FpToString(0.11e-19f));

  EXPECT_EQ("19", FpToString(18.9f, 2));
  EXPECT_EQ("20", FpToString(19.9f, 2));
  EXPECT_EQ("2e+01", FpToString(19.9f, 1));
  EXPECT_EQ("1.2e-05", FpToString(1.234e-5f, 2));
  EXPECT_EQ("1.3e-05", FpToString(1.299e-5f, 2));

  EXPECT_EQ("-1", FpToString(-1.f));
  EXPECT_EQ("-1.25", FpToString(-1.25f));
  EXPECT_EQ("-34.34", FpToString(-34.34f));
  EXPECT_EQ("-1e+20", FpToString(-1e+20f));
  EXPECT_EQ("-1.1e+20", FpToString(-1.1e+20f));
  EXPECT_EQ("-1e-20", FpToString(-1e-20f));
  EXPECT_EQ("-1.1e-20", FpToString(-1.1e-20f));
  EXPECT_EQ("-1e-20", FpToString(-0.1e-19f));
  EXPECT_EQ("-1.1e-20", FpToString(-0.11e-19f));

  EXPECT_EQ("-19", FpToString(-18.9f, 2));
  EXPECT_EQ("-20", FpToString(-19.9f, 2));
  EXPECT_EQ("-2e+01", FpToString(-19.9f, 1));
  EXPECT_EQ("-1.2e-05", FpToString(-1.234e-5f, 2));
  EXPECT_EQ("-1.3e-05", FpToString(-1.299e-5f, 2));
}

}  // namespace
}  // namespace YAML

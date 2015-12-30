#include "yaml-cpp/version.h"

#include "gtest/gtest.h"

namespace YAML {
namespace {
TEST(VersionTest, Major)
{
  ASSERT_EQ(YAML_CPP_VERSION_MAJOR, 0);
}

TEST(VersionTest, Minor)
{
  ASSERT_EQ(YAML_CPP_VERSION_MINOR, 5);
}

TEST(VersionTest, Path)
{
  ASSERT_EQ(YAML_CPP_VERSION_PATCH, 2);
}

TEST(VersionTest, String)
{
  ASSERT_STREQ(YAML_CPP_VERSION, "0.5.2");
}
}
}

#include "yaml-cpp/emitterstyle.h"
#include "yaml-cpp/eventhandler.h"
#include "yaml-cpp/yaml.h"  // IWYU pragma: keep
#include "gtest/gtest.h"

// Base class
class A {
 public:
  A() = default;
  A(int a) : a{a} {}

  // virtual load/emit methods
  virtual void load(const YAML::Node &node) { a = node["a"].as<int>(); }

  virtual YAML::Node emit() const {
    YAML::Node node;
    node["a"] = a;
    return node;
  }

  int a{};
};

// Derived class
class B : public A {
 public:
  B() = default;
  B(int a, int b) : A{a}, b{b} {}

  // override virtual load/emit methods
  virtual void load(const YAML::Node &node) override {
    A::load(node);
    b = node["b"].as<int>();
  }

  virtual YAML::Node emit() const override {
    YAML::Node node = A::emit();
    node["b"] = b;
    return node;
  }

  int b{};
};

// Implementation of convert::{encode,decode} for all classes derived from or
// being A
namespace YAML {
template <typename T>
struct convert<T, typename std::enable_if<std::is_base_of<A, T>::value>::type> {
  static Node encode(const T &rhs) {
    Node node = rhs.emit();
    return node;
  }

  static bool decode(const Node &node, T &rhs) {
    rhs.load(node);
    return true;
  }
};

namespace {

TEST(ConvertPartialSpecializationTest, EncodeBaseClass) {
  Node n(Load("{a: 1}"));
  A a = n.as<A>();
  EXPECT_EQ(a.a, 1);
}

TEST(ConvertPartialSpecializationTest, EncodeDerivedClass) {
  Node n(Load("{a: 1, b: 2}"));
  B b = n.as<B>();
  EXPECT_EQ(b.a, 1);
  EXPECT_EQ(b.b, 2);
}

TEST(ConvertPartialSpecializationTest, DecodeBaseClass) {
  A a(1);
  Node n;
  n = a;
  EXPECT_EQ(a.a, n["a"].as<int>());
}

TEST(ConvertPartialSpecializationTest, DecodeDerivedClass) {
  B b(1, 2);
  Node n;
  n = b;
  EXPECT_EQ(b.a, n["a"].as<int>());
  EXPECT_EQ(b.b, n["b"].as<int>());
}

}  // namespace
}  // namespace YAML
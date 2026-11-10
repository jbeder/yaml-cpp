#include "yaml-cpp/yaml.h"

#include "gtest/gtest.h"

#include <iterator>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace YAML {
namespace {

template <class Iterator, class V>
void CheckIteratorTraits() {
  using Traits = std::iterator_traits<Iterator>;
  EXPECT_TRUE((std::is_same<typename Traits::value_type,
                            detail::iterator_value>::value));
  EXPECT_TRUE((std::is_same<typename Traits::pointer, V*>::value));
  EXPECT_TRUE((std::is_same<typename Traits::reference, V&>::value));
  EXPECT_TRUE((std::is_same<decltype(*std::declval<Iterator&>()), V>::value));
  EXPECT_TRUE((std::is_same<typename Traits::iterator_category,
                            std::bidirectional_iterator_tag>::value));
}

template <class Iterator>
std::vector<typename std::iterator_traits<Iterator>::value_type> Collect(
    Iterator first, Iterator last) {
  return std::vector<typename std::iterator_traits<Iterator>::value_type>(first,
                                                                          last);
}

template <class Iterator>
void CheckSequence(Iterator first, Iterator last,
                   const std::vector<int>& expected) {
  auto values = Collect(first, last);
  ASSERT_EQ(expected.size(), values.size());
  for (std::size_t i = 0; i < values.size(); ++i) {
    EXPECT_EQ(expected[i], values[i].template as<int>());
  }
}

template <class Iterator>
void CheckMap(Iterator first, Iterator last,
              const std::vector<std::pair<std::string, int>>& expected) {
  auto values = Collect(first, last);
  ASSERT_EQ(expected.size(), values.size());
  for (std::size_t i = 0; i < values.size(); ++i) {
    EXPECT_EQ(expected[i].first, values[i].first.template as<std::string>());
    EXPECT_EQ(expected[i].second, values[i].second.template as<int>());
  }
}

TEST(IteratorTest, ValueTypeDoesNotChangeDereferenceConstness) {
  CheckIteratorTraits<iterator, detail::iterator_value>();
  CheckIteratorTraits<const_iterator, const detail::iterator_value>();
  CheckIteratorTraits<reverse_iterator, detail::iterator_value>();
  CheckIteratorTraits<const_reverse_iterator, const detail::iterator_value>();
}

TEST(IteratorTest, CollectSequenceUsingIteratorValueType) {
  Node node = Load("[11, 22, 33]");
  const Node& cn = node;
  const std::vector<int> forward{11, 22, 33};
  const std::vector<int> backward{33, 22, 11};

  CheckSequence(node.begin(), node.end(), forward);
  CheckSequence(cn.begin(), cn.end(), forward);
  CheckSequence(node.rbegin(), node.rend(), backward);
  CheckSequence(cn.rbegin(), cn.rend(), backward);
  EXPECT_EQ(forward, node.as<std::vector<int>>());
}

TEST(IteratorTest, CollectMapUsingIteratorValueType) {
  Node node = Load("{first: 11, second: 22, third: 33}");
  const Node& cn = node;
  const std::vector<std::pair<std::string, int>> forward{
      {"first", 11}, {"second", 22}, {"third", 33}};
  const std::vector<std::pair<std::string, int>> backward{
      {"third", 33}, {"second", 22}, {"first", 11}};

  CheckMap(node.begin(), node.end(), forward);
  CheckMap(cn.begin(), cn.end(), forward);
  CheckMap(node.rbegin(), node.rend(), backward);
  CheckMap(cn.rbegin(), cn.rend(), backward);
  EXPECT_EQ(3, node.size());
  EXPECT_EQ(11, node["first"].as<int>());
  EXPECT_EQ(33, node["third"].as<int>());
}

TEST(IteratorTest, CollectEmptyRangesUsingIteratorValueType) {
  for (NodeType::value type : {NodeType::Undefined, NodeType::Null,
                               NodeType::Sequence, NodeType::Map}) {
    Node node(type);
    const Node& cn = node;
    EXPECT_TRUE(Collect(node.begin(), node.end()).empty());
    EXPECT_TRUE(Collect(cn.begin(), cn.end()).empty());
    EXPECT_TRUE(Collect(node.rbegin(), node.rend()).empty());
    EXPECT_TRUE(Collect(cn.rbegin(), cn.rend()).empty());
  }
}

TEST(IteratorTest, CollectedValuesKeepNodeIdentity) {
  Node sequence = Load("[11, 22]");
  const Node& cn = sequence;
  auto values = Collect(cn.begin(), cn.end());
  ASSERT_EQ(2, values.size());
  EXPECT_TRUE(values[0].is(sequence[0]));
  EXPECT_TRUE(values[1].is(sequence[1]));

  Node map = Load("{first: 11, second: 22}");
  const Node& cm = map;
  auto pairs = Collect(cm.rbegin(), cm.rend());
  ASSERT_EQ(2, pairs.size());
  EXPECT_TRUE(pairs[0].first.is(map.rbegin()->first));
  EXPECT_TRUE(pairs[0].second.is(map["second"]));
  EXPECT_TRUE(pairs[1].second.is(map["first"]));
}

}  // namespace
}  // namespace YAML

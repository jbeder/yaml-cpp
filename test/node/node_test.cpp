#include "yaml-cpp/emitter.h"
#include "yaml-cpp/node/convert.h"
#include "yaml-cpp/node/detail/impl.h"
#include "yaml-cpp/node/emit.h"
#include "yaml-cpp/node/impl.h"
#include "yaml-cpp/node/iterator.h"
#include "yaml-cpp/node/node.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using ::testing::AnyOf;
using ::testing::Eq;

#define EXPECT_THROW_REPRESENTATION_EXCEPTION(statement, message) \
  ASSERT_THROW(statement, RepresentationException);               \
  try {                                                           \
    statement;                                                    \
  } catch (const RepresentationException& e) {                    \
    EXPECT_EQ(e.msg, message);                                    \
  }

namespace YAML {
namespace {
TEST(NodeTest, SimpleScalar) {
  Node node = Node("Hello, World!");
  EXPECT_TRUE(node.IsScalar());
  EXPECT_EQ("Hello, World!", node.as<std::string>());
}

TEST(NodeTest, IntScalar) {
  Node node = Node(15);
  EXPECT_TRUE(node.IsScalar());
  EXPECT_EQ(15, node.as<int>());
}

TEST(NodeTest, SimpleAppendSequence) {
  Node node;
  node.push_back(10);
  node.push_back("foo");
  node.push_back("monkey");
  EXPECT_TRUE(node.IsSequence());
  EXPECT_EQ(3, node.size());
  EXPECT_EQ(10, node[0].as<int>());
  EXPECT_EQ("foo", node[1].as<std::string>());
  EXPECT_EQ("monkey", node[2].as<std::string>());
  EXPECT_TRUE(node.IsSequence());
}

TEST(NodeTest, MapElementRemoval) {
  Node node;
  node["foo"] = "bar";
  node.remove("foo");
  EXPECT_TRUE(!node["foo"]);
}

TEST(NodeTest, SimpleAssignSequence) {
  Node node;
  node[0] = 10;
  node[1] = "foo";
  node[2] = "monkey";
  EXPECT_TRUE(node.IsSequence());
  EXPECT_EQ(3, node.size());
  EXPECT_EQ(10, node[0].as<int>());
  EXPECT_EQ("foo", node[1].as<std::string>());
  EXPECT_EQ("monkey", node[2].as<std::string>());
  EXPECT_TRUE(node.IsSequence());
}

TEST(NodeTest, SimpleMap) {
  Node node;
  node["key"] = "value";
  EXPECT_TRUE(node.IsMap());
  EXPECT_EQ("value", node["key"].as<std::string>());
  EXPECT_EQ(1, node.size());
}

TEST(NodeTest, MapWithUndefinedValues) {
  Node node;
  node["key"] = "value";
  node["undefined"];
  EXPECT_TRUE(node.IsMap());
  EXPECT_EQ("value", node["key"].as<std::string>());
  EXPECT_EQ(1, node.size());

  node["undefined"] = "monkey";
  EXPECT_EQ("monkey", node["undefined"].as<std::string>());
  EXPECT_EQ(2, node.size());
}

TEST(NodeTest, SeqIntoMap) {
  Node node;
  node[0] = "test";
  node[1];
  node[2] = "value";
  EXPECT_TRUE(node.IsMap());
  EXPECT_EQ("test", node[0].as<std::string>());
  EXPECT_EQ("value", node[2].as<std::string>());
  EXPECT_EQ(2, node.size());
}

TEST(NodeTest, RemoveUnassignedNode) {
  Node node(NodeType::Map);
  node["key"];
  node.remove("key");
  EXPECT_EQ(0, node.size());
}

TEST(NodeTest, MapForceInsert) {
  Node node;
  Node k1("k1");
  Node k2("k2");
  Node v1("v1");
  Node v2("v2");
  node[k1] = v1;
  node[k2] = v1;
  EXPECT_TRUE(node.IsMap());
  EXPECT_EQ("v1", node["k1"].as<std::string>());
  EXPECT_EQ("v1", node["k2"].as<std::string>());
  EXPECT_EQ(2, node.size());

  node.force_insert(k2, v2);
  EXPECT_EQ("v1", node["k1"].as<std::string>());
  EXPECT_EQ("v1", node["k2"].as<std::string>());
  EXPECT_EQ(3, node.size());
}

TEST(NodeTest, UndefinedConstNodeWithFallback) {
  Node node;
  const Node& cn = node;
  EXPECT_EQ(cn["undefined"].as<int>(3), 3);
}

TEST(NodeTest, MapIteratorWithUndefinedValues) {
  Node node;
  node["key"] = "value";
  node["undefined"];

  std::size_t count = 0;
  for (const_iterator it = node.begin(); it != node.end(); ++it)
    count++;
  EXPECT_EQ(1, count);
}

TEST(NodeTest, ConstIteratorOnConstUndefinedNode) {
  Node node;
  const Node& cn = node;
  const Node& undefinedCn = cn["undefined"];

  std::size_t count = 0;
  for (const_iterator it = undefinedCn.begin(); it != undefinedCn.end(); ++it) {
    count++;
  }
  EXPECT_EQ(0, count);
}

TEST(NodeTest, IteratorOnConstUndefinedNode) {
  Node node;
  const Node& cn = node;
  const Node& undefinedCn = cn["undefined"];

  Node& nonConstUndefinedNode = const_cast<Node&>(undefinedCn);

  std::size_t count = 0;
  for (iterator it = nonConstUndefinedNode.begin();
       it != nonConstUndefinedNode.end(); ++it) {
    count++;
  }
  EXPECT_EQ(0, count);
}

TEST(NodeTest, SimpleSubkeys) {
  Node node;
  node["device"]["udid"] = "12345";
  node["device"]["name"] = "iPhone";
  node["device"]["os"] = "4.0";
  node["username"] = "monkey";
  EXPECT_EQ("12345", node["device"]["udid"].as<std::string>());
  EXPECT_EQ("iPhone", node["device"]["name"].as<std::string>());
  EXPECT_EQ("4.0", node["device"]["os"].as<std::string>());
  EXPECT_EQ("monkey", node["username"].as<std::string>());
}

TEST(NodeTest, StdArray) {
  std::array<int, 5> evens{{2, 4, 6, 8, 10}};
  Node node;
  node["evens"] = evens;
  std::array<int, 5> actualEvens = node["evens"].as<std::array<int, 5>>();
  EXPECT_EQ(evens, actualEvens);
}

TEST(NodeTest, StdArrayWrongSize) {
  std::array<int, 3> evens{{2, 4, 6}};
  Node node;
  node["evens"] = evens;
  EXPECT_THROW_REPRESENTATION_EXCEPTION(
      (node["evens"].as<std::array<int, 5>>()), ErrorMsg::BAD_CONVERSION);
}

TEST(NodeTest, StdVector) {
  std::vector<int> primes;
  primes.push_back(2);
  primes.push_back(3);
  primes.push_back(5);
  primes.push_back(7);
  primes.push_back(11);
  primes.push_back(13);

  Node node;
  node["primes"] = primes;
  EXPECT_EQ(primes, node["primes"].as<std::vector<int>>());
}

TEST(NodeTest, StdDeque) {
  std::deque<int> primes;
  primes.push_back(2);
  primes.push_back(3);
  primes.push_back(5);
  primes.push_back(7);
  primes.push_back(11);
  primes.push_back(13);

  Node node;
  node["primes"] = primes;
  EXPECT_EQ(primes, node["primes"].as<std::deque<int>>());
}

TEST(NodeTest, StdList) {
  std::list<int> primes;
  primes.push_back(2);
  primes.push_back(3);
  primes.push_back(5);
  primes.push_back(7);
  primes.push_back(11);
  primes.push_back(13);

  Node node;
  node["primes"] = primes;
  EXPECT_EQ(primes, node["primes"].as<std::list<int>>());
}

TEST(NodeTest, StdForwardList) {
  std::forward_list<int> primes;
  primes.push_front(13);
  primes.push_front(11);
  primes.push_front(7);
  primes.push_front(5);
  primes.push_front(3);
  primes.push_front(2);

  Node node;
  node["primes"] = primes;
  EXPECT_EQ(primes, node["primes"].as<std::forward_list<int>>());
}

TEST(NodeTest, StdMap) {
  std::map<int, int> squares;
  squares[0] = 0;
  squares[1] = 1;
  squares[2] = 4;
  squares[3] = 9;
  squares[4] = 16;

  Node node;
  node["squares"] = squares;
  std::map<int, int> actualSquares = node["squares"].as<std::map<int, int>>();
  EXPECT_EQ(squares, actualSquares);
}

TEST(NodeTest, StdUnorderedMap) {
  std::unordered_map<int, int> squares;
  squares[0] = 0;
  squares[1] = 1;
  squares[2] = 4;
  squares[3] = 9;
  squares[4] = 16;

  Node node;
  node["squares"] = squares;
  const auto actualSquares = node["squares"].as<std::unordered_map<int, int>>();
  EXPECT_EQ(squares, actualSquares);
}

TEST(NodeTest, StdSet) {
  std::set<int> primes;
  primes.insert(2);
  primes.insert(3);
  primes.insert(5);
  primes.insert(7);
  primes.insert(11);
  primes.insert(13);

  Node node;
  node["primes"] = primes;
  const auto actualPrimes = node["primes"].as<std::set<int>>();
  EXPECT_EQ(primes, actualPrimes);
}

TEST(NodeTest, StdUnorderedSet) {
  std::unordered_set<int> primes;
  primes.insert(0);
  primes.insert(1);
  primes.insert(4);
  primes.insert(9);
  primes.insert(16);

  Node node;
  node["primes"] = primes;
  const auto actualPrimes = node["primes"].as<std::unordered_set<int>>();
  EXPECT_EQ(primes, actualPrimes);
}

TEST(NodeTest, StdBitset) {
  std::bitset<8> bits;
  bits.set(0, true);
  bits.set(1, true);
  bits.set(2, false);
  bits.set(3, true);
  bits.set(4, true);
  bits.set(5, false);
  bits.set(6, false);
  bits.set(7, false);

  Node node;
  node["bits"] = bits;
  const auto actual = node["bits"].as<std::bitset<8>>();
  EXPECT_EQ(bits, actual);
}

TEST(NodeTest, StdBitsetWrongSize) {
  std::bitset<8> bits;
  bits.set(0, true);
  bits.set(1, true);
  bits.set(2, false);
  bits.set(3, true);
  bits.set(4, true);
  bits.set(5, false);
  bits.set(6, false);
  bits.set(7, false);

  Node node;
  node["bits"] = bits;
  EXPECT_THROW_REPRESENTATION_EXCEPTION((node["bits"].as<std::bitset<1>>()),
                                        ErrorMsg::BAD_CONVERSION);
  EXPECT_THROW_REPRESENTATION_EXCEPTION((node["bits"].as<std::bitset<2>>()),
                                        ErrorMsg::BAD_CONVERSION);
  EXPECT_THROW_REPRESENTATION_EXCEPTION((node["bits"].as<std::bitset<3>>()),
                                        ErrorMsg::BAD_CONVERSION);
  EXPECT_THROW_REPRESENTATION_EXCEPTION((node["bits"].as<std::bitset<4>>()),
                                        ErrorMsg::BAD_CONVERSION);
  EXPECT_THROW_REPRESENTATION_EXCEPTION((node["bits"].as<std::bitset<5>>()),
                                        ErrorMsg::BAD_CONVERSION);
  EXPECT_THROW_REPRESENTATION_EXCEPTION((node["bits"].as<std::bitset<6>>()),
                                        ErrorMsg::BAD_CONVERSION);
  EXPECT_THROW_REPRESENTATION_EXCEPTION((node["bits"].as<std::bitset<7>>()),
                                        ErrorMsg::BAD_CONVERSION);
  EXPECT_THROW_REPRESENTATION_EXCEPTION((node["bits"].as<std::bitset<9>>()),
                                        ErrorMsg::BAD_CONVERSION);
}

TEST(NodeTest, StdBitsetWrongRepresentation) {
  const std::string representation =
      "0011"
      "1000"
      "1101"
      "x000";  // 16 bits, note the wrong character 'x' here
  Node node;
  node["not_really_bits"] = representation;
  EXPECT_THROW_REPRESENTATION_EXCEPTION(
      (node["not_really_bits"].as<std::bitset<16>>()),
      ErrorMsg::BAD_CONVERSION);
}

#define TEST_FROM_TO(TESTNAME, FROM, TO)                                      \
  TEST(NodeTest, TESTNAME) {                                                  \
    const std::FROM primes = {{2, 3, 5, 7, 11, 13}};                          \
    std::size_t count0 = std::distance(std::begin(primes), std::end(primes)); \
    const Node node(primes);                                                  \
    const auto decoded = node.as<std::TO>();                                  \
    auto iter0 = primes.begin();                                              \
    auto iter1 = decoded.begin();                                             \
    std::size_t count1 = 0;                                                   \
    while (iter0 != primes.end() && iter1 != decoded.end()) {                 \
      EXPECT_EQ(*iter0, *iter1);                                              \
      ++iter0;                                                                \
      ++iter1;                                                                \
      ++count1;                                                               \
    }                                                                         \
    EXPECT_EQ(count0, count1);                                                \
    EXPECT_THROW_REPRESENTATION_EXCEPTION((node.as<std::map<int, int>>()),    \
                                          ErrorMsg::BAD_CONVERSION);          \
  }

// needed because the comma would otherwise be seen as the start of a new macro
// argument.
#define ARRAY_TYPE array<int, 6>

TEST_FROM_TO(StdFromVectorToDeque, vector<int>, deque<int>);
TEST_FROM_TO(StdFromVectorToList, vector<int>, list<int>);
TEST_FROM_TO(StdFromVectorToForwardList, vector<int>, forward_list<int>);
TEST_FROM_TO(StdFromVectorToArray, vector<int>, ARRAY_TYPE);

TEST_FROM_TO(StdFromDequeToVector, deque<int>, vector<int>);
TEST_FROM_TO(StdFromDequeToList, deque<int>, list<int>);
TEST_FROM_TO(StdFromDequeToForwardList, deque<int>, forward_list<int>);
TEST_FROM_TO(StdFromDequeToArray, deque<int>, ARRAY_TYPE);

TEST_FROM_TO(StdFromListToVector, list<int>, vector<int>);
TEST_FROM_TO(StdFromListToDeque, list<int>, deque<int>);
TEST_FROM_TO(StdFromListToForwardList, list<int>, forward_list<int>);
TEST_FROM_TO(StdFromListToArray, list<int>, ARRAY_TYPE);

TEST_FROM_TO(StdFromForwardListToVector, forward_list<int>, vector<int>);
TEST_FROM_TO(StdFromForwardListToDeque, forward_list<int>, deque<int>);
TEST_FROM_TO(StdFromForwardListToList, forward_list<int>, list<int>);
TEST_FROM_TO(StdFromForwardListToArray, forward_list<int>, ARRAY_TYPE);

TEST_FROM_TO(StdFromArrayToVector, ARRAY_TYPE, vector<int>);
TEST_FROM_TO(StdFromArrayToDeque, ARRAY_TYPE, deque<int>);
TEST_FROM_TO(StdFromArrayToList, ARRAY_TYPE, list<int>);
TEST_FROM_TO(StdFromArrayToForwardList, ARRAY_TYPE, forward_list<int>);

#undef ARRAY_TYPE
#undef TEST_FROM_TO

TEST(NodeTest, SequenceWithDuplicatesCanBeParsedAsSet) {

  const std::vector<int> withDuplicates{1, 5, 3, 6, 4, 7, 5, 3, 3, 4, 5,
                                        5, 3, 2, 7, 8, 9, 9, 9, 5, 2, 0};

  const auto node = Node(withDuplicates);

  // should succeed -- an std::set is considered a yaml sequence
  const auto withoutDuplicatesAndSorted = node.as<std::set<int>>();
  EXPECT_EQ(withoutDuplicatesAndSorted.size(), 10);
  std::size_t i = 0;
  for (const auto digit : withoutDuplicatesAndSorted) {
    EXPECT_EQ(digit, i);
    ++i;
  }
  EXPECT_EQ(i, 10);
}

TEST(NodeTest, SequenceWithDuplicatesCanBeParsedAsUnorderedSet) {

  const std::vector<int> withDuplicates{1, 5, 3, 6, 4, 7, 5, 3, 3, 4, 5,
                                        5, 3, 2, 7, 8, 9, 9, 9, 5, 2, 0};

  const auto node = Node(withDuplicates);

  // should succeed -- an std::unordered_set is considered a yaml sequence
  const auto withoutDuplicates = node.as<std::unordered_set<int>>();
  EXPECT_EQ(withoutDuplicates.size(), 10);
  int occurences[10];
  std::fill(std::begin(occurences), std::end(occurences), 0);
  for (const auto digit : withoutDuplicates) {
    ++occurences[digit];
  }
  for (std::size_t i = 0; i < 10; ++i)
    EXPECT_EQ(1, occurences[i]);
}

TEST(NodeTest, StdFromMapToUnorderedMap) {
  std::map<std::string, std::vector<std::string>> typeSafeTravisFile{
      {"language", {"c++"}},
      {"os", {"linux", "osx"}},
      {"compiler", {"clang", "gcc"}},
      {"before_install", {"do some stuff", "do some more stuff"}},
      {"before_script", {"mkdir build", "cd build", "cmake .."}},
      {"script", {"make", "test/run-tests"}}};

  const auto node = Node(typeSafeTravisFile);

  // should succeed -- precisely two values in the sequence
  const auto os = node["os"].as<std::pair<std::string, std::string>>();
  EXPECT_EQ(os.first, "linux");
  EXPECT_EQ(os.second, "osx");

  // should succeed -- yaml doesn't know the difference between ordered and
  // unordered.
  using T = std::unordered_map<std::string, std::vector<std::string>>;
  EXPECT_THAT(node.as<T>(),
              testing::UnorderedElementsAreArray(typeSafeTravisFile.begin(),
                                                 typeSafeTravisFile.end()));

  // should fail -- a yaml map is not a yaml sequence
  EXPECT_THROW_REPRESENTATION_EXCEPTION(
      (node.as<  // looks structurally somewhat like a map
          std::vector<std::pair<std::string, std::vector<std::string>>>>()),
      ErrorMsg::BAD_CONVERSION);

  EXPECT_THROW_REPRESENTATION_EXCEPTION(
      (node.as<  // looks structurally somewhat like a map
          std::deque<std::pair<std::string, std::deque<std::string>>>>()),
      ErrorMsg::BAD_CONVERSION);

  EXPECT_THROW_REPRESENTATION_EXCEPTION(
      (node.as<  // looks structurally somewhat like a map
          std::list<std::pair<std::string, std::list<std::string>>>>()),
      ErrorMsg::BAD_CONVERSION);

  EXPECT_THROW_REPRESENTATION_EXCEPTION(
      (node.as<  // looks structurally somewhat like a map
          std::forward_list<
              std::pair<std::string, std::forward_list<std::string>>>>()),
      ErrorMsg::BAD_CONVERSION);
}

TEST(NodeTest, StdFromUnorderedMapToMap) {
  std::unordered_map<std::string, std::vector<std::string>> typeSafeTravisFile{
      {"language", {"c++"}},
      {"os", {"linux", "osx"}},
      {"compiler", {"clang", "gcc"}},
      {"before_install", {"do some stuff", "do some more stuff"}},
      {"before_script", {"mkdir build", "cd build", "cmake .."}},
      {"script", {"make", "test/run-tests"}}};

  const auto node = Node(typeSafeTravisFile);

  // should succeed -- precisely two values in the sequence
  const auto os = node["os"].as<std::pair<std::string, std::string>>();
  EXPECT_EQ(os.first, "linux");
  EXPECT_EQ(os.second, "osx");

  // should succeed -- yaml doesn't know the difference between ordered and
  // unordered.
  using T = std::map<std::string, std::vector<std::string>>;
  EXPECT_THAT(node.as<T>(),
              testing::UnorderedElementsAreArray(typeSafeTravisFile.begin(),
                                                 typeSafeTravisFile.end()));

  // should fail -- a yaml map is not a yaml sequence
  EXPECT_THROW_REPRESENTATION_EXCEPTION(
      (node.as<  // looks structurally somewhat like a map
          std::vector<std::pair<std::string, std::vector<std::string>>>>()),
      ErrorMsg::BAD_CONVERSION);

  EXPECT_THROW_REPRESENTATION_EXCEPTION(
      (node.as<  // looks structurally somewhat like a map
          std::deque<std::pair<std::string, std::deque<std::string>>>>()),
      ErrorMsg::BAD_CONVERSION);

  EXPECT_THROW_REPRESENTATION_EXCEPTION(
      (node.as<  // looks structurally somewhat like a map
          std::list<std::pair<std::string, std::list<std::string>>>>()),
      ErrorMsg::BAD_CONVERSION);

  EXPECT_THROW_REPRESENTATION_EXCEPTION(
      (node.as<  // looks structurally somewhat like a map
          std::forward_list<
              std::pair<std::string, std::forward_list<std::string>>>>()),
      ErrorMsg::BAD_CONVERSION);
}

TEST(NodeTest, StdPair) {
  std::pair<int, std::string> p;
  p.first = 5;
  p.second = "five";

  Node node;
  node["pair"] = p;
  std::pair<int, std::string> actualP =
      node["pair"].as<std::pair<int, std::string>>();
  EXPECT_EQ(p, actualP);
}

TEST(NodeTest, StdPairFailure) {
  std::vector<int> triple{1, 2, 3};
  const auto node = Node(triple);

  // should fail -- an std::pair needs a sequence of length exactly 2, while
  // the Node holds a sequence of length exactly 3.
  EXPECT_THROW_REPRESENTATION_EXCEPTION((node.as<std::pair<int, int>>()),
                                        ErrorMsg::BAD_CONVERSION);
}

TEST(NodeTest, StdTupleSize0) {
  const auto expected = std::tuple<>();
  const auto node = Node(expected);
  const auto actual = node.as<std::tuple<>>();
  EXPECT_EQ(expected, actual);
  EXPECT_EQ(true, node.IsNull());
}

TEST(NodeTest, StdTupleSize1) {
  const auto expected = std::make_tuple(42);
  const auto node = Node(expected);
  const auto actual = node.as<std::remove_cv<decltype(expected)>::type>();
  EXPECT_EQ(expected, actual);

  // should succeed -- tuple is realized as a yaml sequence
  const auto vec = node.as<std::vector<int>>();
  EXPECT_EQ(std::get<0>(expected), vec[0]);
}

TEST(NodeTest, StdTupleSize2) {
  const auto expected = std::make_tuple(42, 3.141592f);
  const auto node = Node(expected);
  const auto actual = node.as<std::remove_cv<decltype(expected)>::type>();
  EXPECT_EQ(expected, actual);

  // should succeed -- tuple is realized as a yaml sequence
  const auto vec = node.as<std::vector<float>>();
  EXPECT_EQ(std::get<0>(expected), vec[0]);
  EXPECT_EQ(std::get<1>(expected), vec[1]);

  // should succeed -- can also be a pair in this case
  const auto pair = node.as<std::pair<int, float>>();
  EXPECT_EQ(std::get<0>(expected), pair.first);
  EXPECT_EQ(std::get<1>(expected), pair.second);
}

TEST(NodeTest, StdTupleSize3) {
  const auto expected = std::make_tuple(42, 3.141592f, std::string("hello"));
  const auto node = Node(expected);
  const auto actual = node.as<std::remove_cv<decltype(expected)>::type>();
  EXPECT_EQ(expected, actual);
}

TEST(NodeTest, StdTupleSize4) {
  const auto expected = std::make_tuple(42, 3.141592f, std::string("hello"),
                                        std::string("world"));
  const auto node = Node(expected);
  const auto actual = node.as<std::remove_cv<decltype(expected)>::type>();
  EXPECT_EQ(expected, actual);
}

TEST(NodeTest, StdTupleSize5) {
  const auto expected =
      std::make_tuple(42, 3.141592f, std::string("hello"), std::string("world"),
                      std::vector<int>{2, 3, 5, 7});
  const auto node = Node(expected);
  const auto actual = node.as<std::remove_cv<decltype(expected)>::type>();
  EXPECT_EQ(expected, actual);
}

TEST(NodeTest, StdTupleSize6) {
  const auto expected =
      std::make_tuple(42, 3.141592f, std::string("hello"), std::string("world"),
                      std::vector<int>{2, 3, 5, 7},
                      std::map<int, int>{{1, 1}, {2, 4}, {3, 9}, {4, 16}});
  const auto node = Node(expected);
  const auto actual = node.as<std::remove_cv<decltype(expected)>::type>();
  EXPECT_EQ(expected, actual);
}

TEST(NodeTest, SimpleAlias) {
  Node node;
  node["foo"] = "value";
  node["bar"] = node["foo"];
  EXPECT_EQ("value", node["foo"].as<std::string>());
  EXPECT_EQ("value", node["bar"].as<std::string>());
  EXPECT_EQ(node["bar"], node["foo"]);
  EXPECT_EQ(2, node.size());
}

TEST(NodeTest, AliasAsKey) {
  Node node;
  node["foo"] = "value";
  Node value = node["foo"];
  node[value] = "foo";
  EXPECT_EQ("value", node["foo"].as<std::string>());
  EXPECT_EQ("foo", node[value].as<std::string>());
  EXPECT_EQ("foo", node["value"].as<std::string>());
  EXPECT_EQ(2, node.size());
}

TEST(NodeTest, SelfReferenceSequence) {
  Node node;
  node[0] = node;
  EXPECT_TRUE(node.IsSequence());
  EXPECT_EQ(1, node.size());
  EXPECT_EQ(node, node[0]);
  EXPECT_EQ(node, node[0][0]);
  EXPECT_EQ(node[0], node[0][0]);
}

TEST(NodeTest, ValueSelfReferenceMap) {
  Node node;
  node["key"] = node;
  EXPECT_TRUE(node.IsMap());
  EXPECT_EQ(1, node.size());
  EXPECT_EQ(node, node["key"]);
  EXPECT_EQ(node, node["key"]["key"]);
  EXPECT_EQ(node["key"], node["key"]["key"]);
}

TEST(NodeTest, KeySelfReferenceMap) {
  Node node;
  node[node] = "value";
  EXPECT_TRUE(node.IsMap());
  EXPECT_EQ(1, node.size());
  EXPECT_EQ("value", node[node].as<std::string>());
}

TEST(NodeTest, SelfReferenceMap) {
  Node node;
  node[node] = node;
  EXPECT_TRUE(node.IsMap());
  EXPECT_EQ(1, node.size());
  EXPECT_EQ(node, node[node]);
  EXPECT_EQ(node, node[node][node]);
  EXPECT_EQ(node[node], node[node][node]);
}

TEST(NodeTest, TempMapVariable) {
  Node node;
  Node tmp = node["key"];
  tmp = "value";
  EXPECT_TRUE(node.IsMap());
  EXPECT_EQ(1, node.size());
  EXPECT_EQ("value", node["key"].as<std::string>());
}

TEST(NodeTest, TempMapVariableAlias) {
  Node node;
  Node tmp = node["key"];
  tmp = node["other"];
  node["other"] = "value";
  EXPECT_TRUE(node.IsMap());
  EXPECT_EQ(2, node.size());
  EXPECT_EQ("value", node["key"].as<std::string>());
  EXPECT_EQ("value", node["other"].as<std::string>());
  EXPECT_EQ(node["key"], node["other"]);
}

TEST(NodeTest, Bool) {
  Node node;
  node[true] = false;
  EXPECT_TRUE(node.IsMap());
  EXPECT_EQ(false, node[true].as<bool>());
}

TEST(NodeTest, AutoBoolConversion) {
#ifdef _MSC_VER
#pragma warning(disable : 4800)
#endif
  Node node;
  node["foo"] = "bar";
  EXPECT_TRUE(static_cast<bool>(node["foo"]));
  EXPECT_TRUE(!node["monkey"]);
  EXPECT_TRUE(!!node["foo"]);
}

TEST(NodeTest, FloatingPrecision) {
  const double x = 0.123456789;
  Node node = Node(x);
  EXPECT_EQ(x, node.as<double>());
}

TEST(NodeTest, SpaceChar) {
  Node node = Node(' ');
  EXPECT_EQ(' ', node.as<char>());
}

TEST(NodeTest, CloneNull) {
  Node node;
  Node clone = Clone(node);
  EXPECT_EQ(NodeType::Null, clone.Type());
}

TEST(NodeTest, KeyNodeExitsScope) {
  Node node;
  {
    Node temp("Hello, world");
    node[temp] = 0;
  }
  for (Node::const_iterator it = node.begin(); it != node.end(); ++it) {
    (void)it;
  }
}

TEST(NodeTest, DefaultNodeStyle) {
  Node node;
  EXPECT_EQ(EmitterStyle::Default, node.Style());
}

TEST(NodeTest, AccessNonexistentKeyOnConstNode) {
  YAML::Node node;
  node["3"] = "4";
  const YAML::Node& other = node;
  ASSERT_FALSE(other["5"]);
}

class NodeEmitterTest : public ::testing::Test {
 protected:
  void ExpectOutput(const std::string& output, const Node& node) {
    Emitter emitter;
    emitter << node;
    ASSERT_TRUE(emitter.good());
    EXPECT_EQ(output, emitter.c_str());
  }

  void ExpectAnyOutput(const Node& node, const std::string& output1,
                       const std::string& output2) {
    Emitter emitter;
    emitter << node;
    ASSERT_TRUE(emitter.good());
    EXPECT_THAT(emitter.c_str(), AnyOf(Eq(output1), Eq(output2)));
  }
};

TEST_F(NodeEmitterTest, SimpleFlowSeqNode) {
  Node node;
  node.SetStyle(EmitterStyle::Flow);
  node.push_back(1.01);
  node.push_back(2.01);
  node.push_back(3.01);

  ExpectOutput("[1.01, 2.01, 3.01]", node);
}

TEST_F(NodeEmitterTest, NestFlowSeqNode) {
  Node node, cell0, cell1;

  cell0.push_back(1.01);
  cell0.push_back(2.01);
  cell0.push_back(3.01);

  cell1.push_back(4.01);
  cell1.push_back(5.01);
  cell1.push_back(6.01);

  node.SetStyle(EmitterStyle::Flow);
  node.push_back(cell0);
  node.push_back(cell1);

  ExpectOutput("[[1.01, 2.01, 3.01], [4.01, 5.01, 6.01]]", node);
}

TEST_F(NodeEmitterTest, MixBlockFlowSeqNode) {
  Node node, cell0, cell1;

  cell0.SetStyle(EmitterStyle::Flow);
  cell0.push_back(1.01);
  cell0.push_back(2.01);
  cell0.push_back(3.01);

  cell1.push_back(4.01);
  cell1.push_back(5.01);
  cell1.push_back(6.01);

  node.SetStyle(EmitterStyle::Block);
  node.push_back(cell0);
  node.push_back(cell1);

  ExpectOutput("- [1.01, 2.01, 3.01]\n-\n  - 4.01\n  - 5.01\n  - 6.01", node);
}

TEST_F(NodeEmitterTest, NestBlockFlowMapListNode) {
  Node node, mapNode, blockNode;

  node.push_back(1.01);
  node.push_back(2.01);
  node.push_back(3.01);

  mapNode.SetStyle(EmitterStyle::Flow);
  mapNode["position"] = node;

  blockNode.push_back(1.01);
  blockNode.push_back(mapNode);

  ExpectOutput("- 1.01\n- {position: [1.01, 2.01, 3.01]}", blockNode);
}

TEST_F(NodeEmitterTest, NestBlockMixMapListNode) {
  Node node, mapNode, blockNode;

  node.push_back(1.01);
  node.push_back(2.01);
  node.push_back(3.01);

  mapNode.SetStyle(EmitterStyle::Flow);
  mapNode["position"] = node;

  blockNode["scalar"] = 1.01;
  blockNode["object"] = mapNode;

  ExpectAnyOutput(blockNode,
                  "scalar: 1.01\nobject: {position: [1.01, 2.01, 3.01]}",
                  "object: {position: [1.01, 2.01, 3.01]}\nscalar: 1.01");
}

TEST_F(NodeEmitterTest, NestBlockMapListNode) {
  Node node, mapNode;

  node.push_back(1.01);
  node.push_back(2.01);
  node.push_back(3.01);

  mapNode.SetStyle(EmitterStyle::Block);
  mapNode["position"] = node;

  ExpectOutput("position:\n  - 1.01\n  - 2.01\n  - 3.01", mapNode);
}

TEST_F(NodeEmitterTest, NestFlowMapListNode) {
  Node node, mapNode;

  node.push_back(1.01);
  node.push_back(2.01);
  node.push_back(3.01);

  mapNode.SetStyle(EmitterStyle::Flow);
  mapNode["position"] = node;

  ExpectOutput("{position: [1.01, 2.01, 3.01]}", mapNode);
}
}
}

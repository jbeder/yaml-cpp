#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include <yaml-cpp/yaml.h>

struct ComparableMark {
  int pos;
  int line, column;

  ComparableMark(int pos, int line, int column)
      : pos(pos), line(line), column(column) {}
};

bool operator==(const YAML::Mark& a, const ComparableMark& b) {
  return a.pos == b.pos && a.line == b.line && a.column == b.column;
}

template <typename Mark>
void PrintMark(const Mark& mark, std::ostream* os) {
  *os << mark.line << ':' << mark.column << " (pos " << mark.pos << ')';
}

void PrintTo(const ComparableMark& mark, std::ostream* os) {
  PrintMark(mark, os);
}

namespace YAML {
void PrintTo(const Mark& mark, std::ostream* os) { PrintMark(mark, os); }
}  // namespace YAML

TEST(CloneNodeTest, PreserveMark) {
  std::string yaml_str = R"(
scalar: value
sequence: [1, 2, 3]
"null": null
[1, 2, 3]: value  # check non-scalar keys
)";

  auto checkMarks = [](const YAML::Node& root_node) {
    EXPECT_EQ(root_node.Mark(), ComparableMark(1, 1, 0));

    const YAML::Node& scalar = root_node["scalar"];
    EXPECT_EQ(scalar.Mark(), ComparableMark(9, 1, 8));

    const YAML::Node& sequence = root_node["sequence"];
    EXPECT_EQ(sequence.Mark(), ComparableMark(25, 2, 10));
    EXPECT_EQ(sequence[0].Mark(), ComparableMark(26, 2, 11));
    EXPECT_EQ(sequence[1].Mark(), ComparableMark(29, 2, 14));
    EXPECT_EQ(sequence[2].Mark(), ComparableMark(32, 2, 17));

    const YAML::Node& null = root_node["null"];
    EXPECT_EQ(null.Mark(), ComparableMark(43, 3, 8));

    YAML::Node sequence_key;
    std::vector<YAML::Mark> key_marks;
    for (auto it = root_node.begin(); it != root_node.end(); ++it) {
      // Not assuming any key order
      key_marks.emplace_back(it->first.Mark());
      if (it->first.IsSequence()) {
        sequence_key.reset(it->first);
      }
    }

    EXPECT_THAT(key_marks,
                testing::UnorderedElementsAre(
                    ComparableMark(1, 1, 0), ComparableMark(15, 2, 0),
                    ComparableMark(35, 3, 0), ComparableMark(48, 4, 0)));

    ASSERT_TRUE(sequence_key);
    EXPECT_EQ(sequence_key[0].Mark(), ComparableMark(49, 4, 1));
    EXPECT_EQ(sequence_key[1].Mark(), ComparableMark(52, 4, 4));
    EXPECT_EQ(sequence_key[2].Mark(), ComparableMark(55, 4, 7));
  };

  YAML::Node root_node = YAML::Load(yaml_str);
  {
    SCOPED_TRACE("original node");
    checkMarks(root_node);
  }
  YAML::Node cloned_node = YAML::Clone(root_node, true);
  {
    SCOPED_TRACE("cloned node");
    checkMarks(cloned_node);
  }
}

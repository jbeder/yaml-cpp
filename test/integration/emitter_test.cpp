#include "yaml-cpp/emitterstyle.h"
#include "yaml-cpp/eventhandler.h"
#include "yaml-cpp/yaml.h"  // IWYU pragma: keep
#include "gtest/gtest.h"

namespace YAML {
namespace {

class NullEventHandler : public EventHandler {
  virtual void OnDocumentStart(const Mark&) {}
  virtual void OnDocumentEnd() {}

  virtual void OnNull(const Mark&, anchor_t) {}
  virtual void OnAlias(const Mark&, anchor_t) {}
  virtual void OnScalar(const Mark&, const std::string&, anchor_t,
                        const std::string&) {}

  virtual void OnSequenceStart(const Mark&, const std::string&, anchor_t,
                               EmitterStyle::value /* style */) {}
  virtual void OnSequenceEnd() {}

  virtual void OnMapStart(const Mark&, const std::string&, anchor_t,
                          EmitterStyle::value /* style */) {}
  virtual void OnMapEnd() {}
};

class EmitterTest : public ::testing::Test {
 protected:
  void ExpectEmit(const std::string& expected) {
    EXPECT_EQ(expected, out.c_str());
    EXPECT_TRUE(out.good()) << "Emitter raised: " << out.GetLastError();
    if (expected == out.c_str()) {
      std::stringstream stream(expected);
      Parser parser;
      NullEventHandler handler;
      parser.HandleNextDocument(handler);
    }
  }

  Emitter out;
};

TEST_F(EmitterTest, SimpleScalar) {
  out << "Hello, World!";

  ExpectEmit("Hello, World!");
}

TEST_F(EmitterTest, SimpleQuotedScalar) {
  Node n(Load("\"test\""));
  out << n;
  ExpectEmit("test");
}

TEST_F(EmitterTest, DumpAndSize) {
  Node n(Load("test"));
  EXPECT_EQ("test", Dump(n));
  out << n;
  EXPECT_EQ(4, out.size());
}

TEST_F(EmitterTest, NullScalar) {
  Node n(Load("null"));
  out << n;
  ExpectEmit("~");
}

TEST_F(EmitterTest, AliasScalar) {
  Node n(Load("[&a str, *a]"));
  out << n;
  ExpectEmit("[&1 str, *1]");
}

TEST_F(EmitterTest, StringFormat) {
  out << EMITTER_MANIP::BeginSeq;
  out.SetStringFormat(EMITTER_MANIP::SingleQuoted);
  out << "string";
  out.SetStringFormat(EMITTER_MANIP::DoubleQuoted);
  out << "string";
  out.SetStringFormat(EMITTER_MANIP::Literal);
  out << "string";
  out << EMITTER_MANIP::EndSeq;

  ExpectEmit("- 'string'\n- \"string\"\n- |\n  string");
}

TEST_F(EmitterTest, IntBase) {
  out << EMITTER_MANIP::BeginSeq;
  out.SetIntBase(EMITTER_MANIP::Dec);
  out << 1024;
  out.SetIntBase(EMITTER_MANIP::Hex);
  out << 1024;
  out.SetIntBase(EMITTER_MANIP::Oct);
  out << 1024;
  out << EMITTER_MANIP::EndSeq;

  ExpectEmit("- 1024\n- 0x400\n- 02000");
}

TEST_F(EmitterTest, NumberPrecision) {
  out.SetFloatPrecision(3);
  out.SetDoublePrecision(2);
  out << EMITTER_MANIP::BeginSeq;
  out << 3.1425926f;
  out << 53.5893;
  out << 2384626.4338;
  out << EMITTER_MANIP::EndSeq;

  ExpectEmit("- 3.14\n- 54\n- 2.4e+06");
}

TEST_F(EmitterTest, SimpleSeq) {
  out << EMITTER_MANIP::BeginSeq;
  out << "eggs";
  out << "bread";
  out << "milk";
  out << EMITTER_MANIP::EndSeq;

  ExpectEmit("- eggs\n- bread\n- milk");
}

TEST_F(EmitterTest, SimpleFlowSeq) {
  out << EMITTER_MANIP::Flow;
  out << EMITTER_MANIP::BeginSeq;
  out << "Larry";
  out << "Curly";
  out << "Moe";
  out << EMITTER_MANIP::EndSeq;

  ExpectEmit("[Larry, Curly, Moe]");
}

TEST_F(EmitterTest, EmptyFlowSeq) {
  out << EMITTER_MANIP::Flow;
  out << EMITTER_MANIP::BeginSeq;
  out << EMITTER_MANIP::EndSeq;

  ExpectEmit("[]");
}

TEST_F(EmitterTest, EmptyBlockSeqWithBegunContent) {
  out << EMITTER_MANIP::BeginSeq;
  out << EMITTER_MANIP::BeginSeq << Comment("comment") << EMITTER_MANIP::EndSeq;
  out << EMITTER_MANIP::BeginSeq << EMITTER_MANIP::Newline << EMITTER_MANIP::EndSeq;
  out << EMITTER_MANIP::EndSeq;

  ExpectEmit(R"(-
# comment
  []
-

  [])");
}

TEST_F(EmitterTest, EmptyBlockMapWithBegunContent) {
  out << EMITTER_MANIP::BeginSeq;
  out << EMITTER_MANIP::BeginMap << Comment("comment") << EMITTER_MANIP::EndMap;
  out << EMITTER_MANIP::BeginMap << EMITTER_MANIP::Newline << EMITTER_MANIP::EndMap;
  out << EMITTER_MANIP::EndSeq;

  ExpectEmit(R"(-  # comment
  {}
-
  {})");
}

TEST_F(EmitterTest, EmptyFlowSeqWithBegunContent) {
  out << EMITTER_MANIP::Flow;
  out << EMITTER_MANIP::BeginSeq;
  out << EMITTER_MANIP::BeginSeq << Comment("comment") << EMITTER_MANIP::EndSeq;
  out << EMITTER_MANIP::BeginSeq << EMITTER_MANIP::Newline << EMITTER_MANIP::EndSeq;
  out << EMITTER_MANIP::EndSeq;

  ExpectEmit(R"([[  # comment
  ], [
  ]])");
}

TEST_F(EmitterTest, EmptyFlowMapWithBegunContent) {
  out << EMITTER_MANIP::Flow;
  out << EMITTER_MANIP::BeginSeq;
  out << EMITTER_MANIP::BeginMap << Comment("comment") << EMITTER_MANIP::EndMap;
  out << EMITTER_MANIP::BeginMap << EMITTER_MANIP::Newline << EMITTER_MANIP::EndMap;
  out << EMITTER_MANIP::EndSeq;

  ExpectEmit(R"([{  # comment
  }, {
  }])");
}

TEST_F(EmitterTest, NestedBlockSeq) {
  out << EMITTER_MANIP::BeginSeq;
  out << "item 1";
  out << EMITTER_MANIP::BeginSeq << "subitem 1"
      << "subitem 2" << EMITTER_MANIP::EndSeq;
  out << EMITTER_MANIP::EndSeq;

  ExpectEmit("- item 1\n-\n  - subitem 1\n  - subitem 2");
}

TEST_F(EmitterTest, NestedFlowSeq) {
  out << EMITTER_MANIP::BeginSeq;
  out << "one";
  out << EMITTER_MANIP::Flow << EMITTER_MANIP::BeginSeq << "two"
      << "three" << EMITTER_MANIP::EndSeq;
  out << EMITTER_MANIP::EndSeq;

  ExpectEmit("- one\n- [two, three]");
}

TEST_F(EmitterTest, SimpleMap) {
  out << EMITTER_MANIP::BeginMap;
  out << EMITTER_MANIP::Key << "name";
  out << EMITTER_MANIP::Value << "Ryan Braun";
  out << EMITTER_MANIP::Key << "position";
  out << EMITTER_MANIP::Value << "3B";
  out << EMITTER_MANIP::EndMap;

  ExpectEmit("name: Ryan Braun\nposition: 3B");
}

TEST_F(EmitterTest, SimpleFlowMap) {
  out << EMITTER_MANIP::Flow;
  out << EMITTER_MANIP::BeginMap;
  out << EMITTER_MANIP::Key << "shape";
  out << EMITTER_MANIP::Value << "square";
  out << EMITTER_MANIP::Key << "color";
  out << EMITTER_MANIP::Value << "blue";
  out << EMITTER_MANIP::EndMap;

  ExpectEmit("{shape: square, color: blue}");
}

TEST_F(EmitterTest, MapAndList) {
  out << EMITTER_MANIP::BeginMap;
  out << EMITTER_MANIP::Key << "name";
  out << EMITTER_MANIP::Value << "Barack Obama";
  out << EMITTER_MANIP::Key << "children";
  out << EMITTER_MANIP::Value << EMITTER_MANIP::BeginSeq << "Sasha"
      << "Malia" << EMITTER_MANIP::EndSeq;
  out << EMITTER_MANIP::EndMap;

  ExpectEmit("name: Barack Obama\nchildren:\n  - Sasha\n  - Malia");
}

TEST_F(EmitterTest, ListAndMap) {
  out << EMITTER_MANIP::BeginSeq;
  out << "item 1";
  out << EMITTER_MANIP::BeginMap;
  out << EMITTER_MANIP::Key << "pens" << EMITTER_MANIP::Value << 8;
  out << EMITTER_MANIP::Key << "pencils" << EMITTER_MANIP::Value << 14;
  out << EMITTER_MANIP::EndMap;
  out << "item 2";
  out << EMITTER_MANIP::EndSeq;

  ExpectEmit("- item 1\n- pens: 8\n  pencils: 14\n- item 2");
}

TEST_F(EmitterTest, NestedBlockMap) {
  out << EMITTER_MANIP::BeginMap;
  out << EMITTER_MANIP::Key << "name";
  out << EMITTER_MANIP::Value << "Fred";
  out << EMITTER_MANIP::Key << "grades";
  out << EMITTER_MANIP::Value;
  out << EMITTER_MANIP::BeginMap;
  out << EMITTER_MANIP::Key << "algebra" << EMITTER_MANIP::Value << "A";
  out << EMITTER_MANIP::Key << "physics" << EMITTER_MANIP::Value << "C+";
  out << EMITTER_MANIP::Key << "literature" << EMITTER_MANIP::Value << "B";
  out << EMITTER_MANIP::EndMap;
  out << EMITTER_MANIP::EndMap;

  ExpectEmit(
      "name: Fred\ngrades:\n  algebra: A\n  physics: C+\n  literature: B");
}

TEST_F(EmitterTest, NestedFlowMap) {
  out << EMITTER_MANIP::Flow;
  out << EMITTER_MANIP::BeginMap;
  out << EMITTER_MANIP::Key << "name";
  out << EMITTER_MANIP::Value << "Fred";
  out << EMITTER_MANIP::Key << "grades";
  out << EMITTER_MANIP::Value;
  out << EMITTER_MANIP::BeginMap;
  out << EMITTER_MANIP::Key << "algebra" << EMITTER_MANIP::Value << "A";
  out << EMITTER_MANIP::Key << "physics" << EMITTER_MANIP::Value << "C+";
  out << EMITTER_MANIP::Key << "literature" << EMITTER_MANIP::Value << "B";
  out << EMITTER_MANIP::EndMap;
  out << EMITTER_MANIP::EndMap;

  ExpectEmit("{name: Fred, grades: {algebra: A, physics: C+, literature: B}}");
}

TEST_F(EmitterTest, MapListMix) {
  out << EMITTER_MANIP::BeginMap;
  out << EMITTER_MANIP::Key << "name";
  out << EMITTER_MANIP::Value << "Bob";
  out << EMITTER_MANIP::Key << "position";
  out << EMITTER_MANIP::Value;
  out << EMITTER_MANIP::Flow << EMITTER_MANIP::BeginSeq << 2 << 4 << EMITTER_MANIP::EndSeq;
  out << EMITTER_MANIP::Key << "invincible" << EMITTER_MANIP::Value << EMITTER_MANIP::OnOffBool << false;
  out << EMITTER_MANIP::EndMap;

  ExpectEmit("name: Bob\nposition: [2, 4]\ninvincible: off");
}

TEST_F(EmitterTest, SimpleLongKey) {
  out << EMITTER_MANIP::LongKey;
  out << EMITTER_MANIP::BeginMap;
  out << EMITTER_MANIP::Key << "height";
  out << EMITTER_MANIP::Value << "5'9\"";
  out << EMITTER_MANIP::Key << "weight";
  out << EMITTER_MANIP::Value << 145;
  out << EMITTER_MANIP::EndMap;

  ExpectEmit("? height\n: 5'9\"\n? weight\n: 145");
}

TEST_F(EmitterTest, SingleLongKey) {
  const std::string shortKey(1024, 'a');
  const std::string longKey(1025, 'a');
  out << EMITTER_MANIP::BeginMap;
  out << EMITTER_MANIP::Key << "age";
  out << EMITTER_MANIP::Value << "24";
  out << EMITTER_MANIP::LongKey << EMITTER_MANIP::Key << "height";
  out << EMITTER_MANIP::Value << "5'9\"";
  out << EMITTER_MANIP::Key << "weight";
  out << EMITTER_MANIP::Value << 145;
  out << EMITTER_MANIP::Key << shortKey;
  out << EMITTER_MANIP::Value << "1";
  out << EMITTER_MANIP::Key << longKey;
  out << EMITTER_MANIP::Value << "1";
  out << EMITTER_MANIP::EndMap;

  ExpectEmit("age: 24\n? height\n: 5'9\"\nweight: 145\n" + shortKey +
             ": 1\n? " + longKey + "\n: 1");
}

TEST_F(EmitterTest, ComplexLongKey) {
  out << EMITTER_MANIP::LongKey;
  out << EMITTER_MANIP::BeginMap;
  out << EMITTER_MANIP::Key << EMITTER_MANIP::BeginSeq << 1 << 3 << EMITTER_MANIP::EndSeq;
  out << EMITTER_MANIP::Value << "monster";
  out << EMITTER_MANIP::Key << EMITTER_MANIP::Flow << EMITTER_MANIP::BeginSeq << 2 << 0 << EMITTER_MANIP::EndSeq;
  out << EMITTER_MANIP::Value << "demon";
  out << EMITTER_MANIP::EndMap;

  ExpectEmit("? - 1\n  - 3\n: monster\n? [2, 0]\n: demon");
}

TEST_F(EmitterTest, AutoLongKey) {
  out << EMITTER_MANIP::BeginMap;
  out << EMITTER_MANIP::Key << EMITTER_MANIP::BeginSeq << 1 << 3 << EMITTER_MANIP::EndSeq;
  out << EMITTER_MANIP::Value << "monster";
  out << EMITTER_MANIP::Key << EMITTER_MANIP::Flow << EMITTER_MANIP::BeginSeq << 2 << 0 << EMITTER_MANIP::EndSeq;
  out << EMITTER_MANIP::Value << "demon";
  out << EMITTER_MANIP::Key << "the origin";
  out << EMITTER_MANIP::Value << "angel";
  out << EMITTER_MANIP::EndMap;

  ExpectEmit("? - 1\n  - 3\n: monster\n[2, 0]: demon\nthe origin: angel");
}

TEST_F(EmitterTest, ScalarFormat) {
  out << EMITTER_MANIP::BeginSeq;
  out << "simple scalar";
  out << EMITTER_MANIP::SingleQuoted << "explicit single-quoted scalar";
  out << EMITTER_MANIP::DoubleQuoted << "explicit double-quoted scalar";
  out << "auto-detected\ndouble-quoted scalar";
  out << "a non-\"auto-detected\" double-quoted scalar";
  out << EMITTER_MANIP::Literal
      << "literal scalar\nthat may span\nmany, many\nlines "
         "and have \"whatever\" crazy\tsymbols that we like";
  out << EMITTER_MANIP::EndSeq;

  ExpectEmit(
      "- simple scalar\n- 'explicit single-quoted scalar'\n- \"explicit "
      "double-quoted scalar\"\n- \"auto-detected\\ndouble-quoted "
      "scalar\"\n- a "
      "non-\"auto-detected\" double-quoted scalar\n- |\n  literal scalar\n "
      " "
      "that may span\n  many, many\n  lines and have \"whatever\" "
      "crazy\tsymbols that we like");
}

TEST_F(EmitterTest, AutoLongKeyScalar) {
  out << EMITTER_MANIP::BeginMap;
  out << EMITTER_MANIP::Key << EMITTER_MANIP::Literal << "multi-line\nscalar";
  out << EMITTER_MANIP::Value << "and its value";
  out << EMITTER_MANIP::EndMap;

  ExpectEmit("? |\n  multi-line\n  scalar\n: and its value");
}

TEST_F(EmitterTest, LongKeyFlowMap) {
  out << EMITTER_MANIP::Flow;
  out << EMITTER_MANIP::BeginMap;
  out << EMITTER_MANIP::Key << "simple key";
  out << EMITTER_MANIP::Value << "and value";
  out << EMITTER_MANIP::LongKey << EMITTER_MANIP::Key << "long key";
  out << EMITTER_MANIP::Value << "and its value";
  out << EMITTER_MANIP::EndMap;

  ExpectEmit("{simple key: and value, ? long key: and its value}");
}

TEST_F(EmitterTest, BlockMapAsKey) {
  out << EMITTER_MANIP::BeginMap;
  out << EMITTER_MANIP::Key;
  out << EMITTER_MANIP::BeginMap;
  out << EMITTER_MANIP::Key << "key" << EMITTER_MANIP::Value << "value";
  out << EMITTER_MANIP::Key << "next key" << EMITTER_MANIP::Value << "next value";
  out << EMITTER_MANIP::EndMap;
  out << EMITTER_MANIP::Value;
  out << "total value";
  out << EMITTER_MANIP::EndMap;

  ExpectEmit("? key: value\n  next key: next value\n: total value");
}

TEST_F(EmitterTest, AliasAndAnchor) {
  out << EMITTER_MANIP::BeginSeq;
  out << Anchor("fred");
  out << EMITTER_MANIP::BeginMap;
  out << EMITTER_MANIP::Key << "name" << EMITTER_MANIP::Value << "Fred";
  out << EMITTER_MANIP::Key << "age" << EMITTER_MANIP::Value << 42;
  out << EMITTER_MANIP::EndMap;
  out << Alias("fred");
  out << EMITTER_MANIP::EndSeq;

  ExpectEmit("- &fred\n  name: Fred\n  age: 42\n- *fred");
}

TEST_F(EmitterTest, AliasOnKey) {
  out << EMITTER_MANIP::BeginSeq;
  out << Anchor("name") << "Name";
  out << EMITTER_MANIP::BeginMap;
  out << EMITTER_MANIP::Key << Alias("name") << EMITTER_MANIP::Value << "Fred";
  out << EMITTER_MANIP::EndMap;
  out << EMITTER_MANIP::Flow << EMITTER_MANIP::BeginMap;
  out << EMITTER_MANIP::Key << Alias("name") << EMITTER_MANIP::Value << "Mike";
  out << EMITTER_MANIP::EndMap;
  out << EMITTER_MANIP::EndSeq;
  ExpectEmit(R"(- &name Name
- *name : Fred
- {*name : Mike})");
}

TEST_F(EmitterTest, AliasAndAnchorWithNull) {
  out << EMITTER_MANIP::BeginSeq;
  out << Anchor("fred") << Null;
  out << Alias("fred");
  out << EMITTER_MANIP::EndSeq;

  ExpectEmit("- &fred ~\n- *fred");
}

TEST_F(EmitterTest, AliasAndAnchorInFlow) {
  out << EMITTER_MANIP::Flow << EMITTER_MANIP::BeginSeq;
  out << Anchor("fred");
  out << EMITTER_MANIP::BeginMap;
  out << EMITTER_MANIP::Key << "name" << EMITTER_MANIP::Value << "Fred";
  out << EMITTER_MANIP::Key << "age" << EMITTER_MANIP::Value << 42;
  out << EMITTER_MANIP::EndMap;
  out << Alias("fred");
  out << EMITTER_MANIP::EndSeq;

  ExpectEmit("[&fred {name: Fred, age: 42}, *fred]");
}

TEST_F(EmitterTest, SimpleVerbatimTag) {
  out << VerbatimTag("!foo") << "bar";

  ExpectEmit("!<!foo> bar");
}

TEST_F(EmitterTest, VerbatimTagInBlockSeq) {
  out << EMITTER_MANIP::BeginSeq;
  out << VerbatimTag("!foo") << "bar";
  out << "baz";
  out << EMITTER_MANIP::EndSeq;

  ExpectEmit("- !<!foo> bar\n- baz");
}

TEST_F(EmitterTest, VerbatimTagInFlowSeq) {
  out << EMITTER_MANIP::Flow << EMITTER_MANIP::BeginSeq;
  out << VerbatimTag("!foo") << "bar";
  out << "baz";
  out << EMITTER_MANIP::EndSeq;

  ExpectEmit("[!<!foo> bar, baz]");
}

TEST_F(EmitterTest, VerbatimTagInFlowSeqWithNull) {
  out << EMITTER_MANIP::Flow << EMITTER_MANIP::BeginSeq;
  out << VerbatimTag("!foo") << Null;
  out << "baz";
  out << EMITTER_MANIP::EndSeq;

  ExpectEmit("[!<!foo> ~, baz]");
}

TEST_F(EmitterTest, VerbatimTagInBlockMap) {
  out << EMITTER_MANIP::BeginMap;
  out << EMITTER_MANIP::Key << VerbatimTag("!foo") << "bar";
  out << EMITTER_MANIP::Value << VerbatimTag("!waz") << "baz";
  out << EMITTER_MANIP::EndMap;

  ExpectEmit("!<!foo> bar: !<!waz> baz");
}

TEST_F(EmitterTest, VerbatimTagInFlowMap) {
  out << EMITTER_MANIP::Flow << EMITTER_MANIP::BeginMap;
  out << EMITTER_MANIP::Key << VerbatimTag("!foo") << "bar";
  out << EMITTER_MANIP::Value << "baz";
  out << EMITTER_MANIP::EndMap;

  ExpectEmit("{!<!foo> bar: baz}");
}

TEST_F(EmitterTest, VerbatimTagInFlowMapWithNull) {
  out << EMITTER_MANIP::Flow << EMITTER_MANIP::BeginMap;
  out << EMITTER_MANIP::Key << VerbatimTag("!foo") << Null;
  out << EMITTER_MANIP::Value << "baz";
  out << EMITTER_MANIP::EndMap;

  ExpectEmit("{!<!foo> ~: baz}");
}

TEST_F(EmitterTest, VerbatimTagWithEmptySeq) {
  out << VerbatimTag("!foo") << EMITTER_MANIP::BeginSeq << EMITTER_MANIP::EndSeq;

  ExpectEmit("!<!foo>\n[]");
}

TEST_F(EmitterTest, VerbatimTagWithEmptyMap) {
  out << VerbatimTag("!bar") << EMITTER_MANIP::BeginMap << EMITTER_MANIP::EndMap;

  ExpectEmit("!<!bar>\n{}");
}

TEST_F(EmitterTest, VerbatimTagWithEmptySeqAndMap) {
  out << EMITTER_MANIP::BeginSeq;
  out << VerbatimTag("!foo") << EMITTER_MANIP::BeginSeq << EMITTER_MANIP::EndSeq;
  out << VerbatimTag("!bar") << EMITTER_MANIP::BeginMap << EMITTER_MANIP::EndMap;
  out << EMITTER_MANIP::EndSeq;

  ExpectEmit("- !<!foo>\n  []\n- !<!bar>\n  {}");
}

TEST_F(EmitterTest, ByKindTagWithScalar) {
  out << EMITTER_MANIP::BeginSeq;
  out << EMITTER_MANIP::DoubleQuoted << "12";
  out << "12";
  out << EMITTER_MANIP::TagByKind << "12";
  out << EMITTER_MANIP::EndSeq;

  ExpectEmit("- \"12\"\n- 12\n- ! 12");
}

TEST_F(EmitterTest, LocalTagInNameHandle) {
  out << LocalTag("a", "foo") << "bar";

  ExpectEmit("!a!foo bar");
}

TEST_F(EmitterTest, LocalTagWithScalar) {
  out << LocalTag("foo") << "bar";

  ExpectEmit("!foo bar");
}

TEST_F(EmitterTest, ComplexDoc) {
  out << EMITTER_MANIP::BeginMap;
  out << EMITTER_MANIP::Key << "receipt";
  out << EMITTER_MANIP::Value << "Oz-Ware Purchase Invoice";
  out << EMITTER_MANIP::Key << "date";
  out << EMITTER_MANIP::Value << "2007-08-06";
  out << EMITTER_MANIP::Key << "customer";
  out << EMITTER_MANIP::Value;
  out << EMITTER_MANIP::BeginMap;
  out << EMITTER_MANIP::Key << "given";
  out << EMITTER_MANIP::Value << "Dorothy";
  out << EMITTER_MANIP::Key << "family";
  out << EMITTER_MANIP::Value << "Gale";
  out << EMITTER_MANIP::EndMap;
  out << EMITTER_MANIP::Key << "items";
  out << EMITTER_MANIP::Value;
  out << EMITTER_MANIP::BeginSeq;
  out << EMITTER_MANIP::BeginMap;
  out << EMITTER_MANIP::Key << "part_no";
  out << EMITTER_MANIP::Value << "A4786";
  out << EMITTER_MANIP::Key << "descrip";
  out << EMITTER_MANIP::Value << "Water Bucket (Filled)";
  out << EMITTER_MANIP::Key << "price";
  out << EMITTER_MANIP::Value << 1.47;
  out << EMITTER_MANIP::Key << "quantity";
  out << EMITTER_MANIP::Value << 4;
  out << EMITTER_MANIP::EndMap;
  out << EMITTER_MANIP::BeginMap;
  out << EMITTER_MANIP::Key << "part_no";
  out << EMITTER_MANIP::Value << "E1628";
  out << EMITTER_MANIP::Key << "descrip";
  out << EMITTER_MANIP::Value << "High Heeled \"Ruby\" Slippers";
  out << EMITTER_MANIP::Key << "price";
  out << EMITTER_MANIP::Value << 100.27;
  out << EMITTER_MANIP::Key << "quantity";
  out << EMITTER_MANIP::Value << 1;
  out << EMITTER_MANIP::EndMap;
  out << EMITTER_MANIP::EndSeq;
  out << EMITTER_MANIP::Key << "bill-to";
  out << EMITTER_MANIP::Value << Anchor("id001");
  out << EMITTER_MANIP::BeginMap;
  out << EMITTER_MANIP::Key << "street";
  out << EMITTER_MANIP::Value << EMITTER_MANIP::Literal << "123 Tornado Alley\nSuite 16";
  out << EMITTER_MANIP::Key << "city";
  out << EMITTER_MANIP::Value << "East Westville";
  out << EMITTER_MANIP::Key << "state";
  out << EMITTER_MANIP::Value << "KS";
  out << EMITTER_MANIP::EndMap;
  out << EMITTER_MANIP::Key << "ship-to";
  out << EMITTER_MANIP::Value << Alias("id001");
  out << EMITTER_MANIP::EndMap;

  ExpectEmit(
      "receipt: Oz-Ware Purchase Invoice\ndate: 2007-08-06\ncustomer:\n  "
      "given: Dorothy\n  family: Gale\nitems:\n  - part_no: A4786\n    "
      "descrip: Water Bucket (Filled)\n    price: 1.47\n    quantity: 4\n  - "
      "part_no: E1628\n    descrip: High Heeled \"Ruby\" Slippers\n    price: "
      "100.27\n    quantity: 1\nbill-to: &id001\n  street: |\n    123 Tornado "
      "Alley\n    Suite 16\n  city: East Westville\n  state: KS\nship-to: "
      "*id001");
}

TEST_F(EmitterTest, STLContainers) {
  out << EMITTER_MANIP::BeginSeq;
  std::vector<int> primes;
  primes.push_back(2);
  primes.push_back(3);
  primes.push_back(5);
  primes.push_back(7);
  primes.push_back(11);
  primes.push_back(13);
  out << EMITTER_MANIP::Flow << primes;
  std::map<std::string, int> ages;
  ages["Daniel"] = 26;
  ages["Jesse"] = 24;
  out << ages;
  out << EMITTER_MANIP::EndSeq;

  ExpectEmit("- [2, 3, 5, 7, 11, 13]\n- Daniel: 26\n  Jesse: 24");
}

TEST_F(EmitterTest, CommentStyle) {
  out.SetPreCommentIndent(1);
  out.SetPostCommentIndent(2);
  out << EMITTER_MANIP::BeginMap;
  out << EMITTER_MANIP::Key << "method";
  out << EMITTER_MANIP::Value << "least squares" << Comment("should we change this method?");
  out << EMITTER_MANIP::EndMap;

  ExpectEmit("method: least squares #  should we change this method?");
}

TEST_F(EmitterTest, SimpleComment) {
  out << EMITTER_MANIP::BeginMap;
  out << EMITTER_MANIP::Key << "method";
  out << EMITTER_MANIP::Value << "least squares" << Comment("should we change this method?");
  out << EMITTER_MANIP::EndMap;

  ExpectEmit("method: least squares  # should we change this method?");
}

TEST_F(EmitterTest, MultiLineComment) {
  out << EMITTER_MANIP::BeginSeq;
  out << "item 1"
      << Comment(
             "really really long\ncomment that couldn't "
             "possibly\nfit on one line");
  out << "item 2";
  out << EMITTER_MANIP::EndSeq;

  ExpectEmit(
      "- item 1  # really really long\n          # comment that couldn't "
      "possibly\n          # fit on one line\n- item 2");
}

TEST_F(EmitterTest, ComplexComments) {
  out << EMITTER_MANIP::BeginMap;
  out << EMITTER_MANIP::LongKey << EMITTER_MANIP::Key << "long key" << Comment("long key");
  out << EMITTER_MANIP::Value << "value";
  out << EMITTER_MANIP::EndMap;

  ExpectEmit("? long key  # long key\n: value");
}

TEST_F(EmitterTest, InitialComment) {
  out << Comment("A comment describing the purpose of the file.");
  out << EMITTER_MANIP::BeginMap << EMITTER_MANIP::Key << "key" << EMITTER_MANIP::Value << "value" << EMITTER_MANIP::EndMap;

  ExpectEmit("# A comment describing the purpose of the file.\nkey: value");
}

TEST_F(EmitterTest, InitialCommentWithDocIndicator) {
  out << EMITTER_MANIP::BeginDoc << Comment("A comment describing the purpose of the file.");
  out << EMITTER_MANIP::BeginMap << EMITTER_MANIP::Key << "key" << EMITTER_MANIP::Value << "value" << EMITTER_MANIP::EndMap;

  ExpectEmit(
      "---\n# A comment describing the purpose of the file.\nkey: value");
}

TEST_F(EmitterTest, CommentInFlowSeq) {
  out << EMITTER_MANIP::Flow << EMITTER_MANIP::BeginSeq << "foo" << Comment("foo!") << "bar" << EMITTER_MANIP::EndSeq;

  ExpectEmit("[foo,  # foo!\nbar]");
}

TEST_F(EmitterTest, CommentInFlowMap) {
  out << EMITTER_MANIP::Flow << EMITTER_MANIP::BeginMap;
  out << EMITTER_MANIP::Key << "foo" << EMITTER_MANIP::Value << "foo value";
  out << EMITTER_MANIP::Key << "bar" << EMITTER_MANIP::Value << "bar value" << Comment("bar!");
  out << EMITTER_MANIP::Key << "baz" << EMITTER_MANIP::Value << "baz value" << Comment("baz!");
  out << EMITTER_MANIP::EndMap;

  ExpectEmit(
      "{foo: foo value, bar: bar value,  # bar!\nbaz: baz value,  # baz!\n}");
}

TEST_F(EmitterTest, Indentation) {
  out << Indent(4);
  out << EMITTER_MANIP::BeginSeq;
  out << EMITTER_MANIP::BeginMap;
  out << EMITTER_MANIP::Key << "key 1" << EMITTER_MANIP::Value << "value 1";
  out << EMITTER_MANIP::Key << "key 2" << EMITTER_MANIP::Value << EMITTER_MANIP::BeginSeq << "a"
      << "b"
      << "c" << EMITTER_MANIP::EndSeq;
  out << EMITTER_MANIP::EndMap;
  out << EMITTER_MANIP::EndSeq;

  ExpectEmit(
      "-   key 1: value 1\n    key 2:\n        -   a\n        -   b\n        - "
      "  c");
}

TEST_F(EmitterTest, SimpleGlobalSettings) {
  out.SetIndent(4);
  out.SetMapFormat(EMITTER_MANIP::LongKey);

  out << EMITTER_MANIP::BeginSeq;
  out << EMITTER_MANIP::BeginMap;
  out << EMITTER_MANIP::Key << "key 1" << EMITTER_MANIP::Value << "value 1";
  out << EMITTER_MANIP::Key << "key 2" << EMITTER_MANIP::Value << EMITTER_MANIP::Flow << EMITTER_MANIP::BeginSeq << "a"
      << "b"
      << "c" << EMITTER_MANIP::EndSeq;
  out << EMITTER_MANIP::EndMap;
  out << EMITTER_MANIP::EndSeq;

  ExpectEmit("-   ? key 1\n    : value 1\n    ? key 2\n    : [a, b, c]");
}

TEST_F(EmitterTest, GlobalLongKeyOnSeq) {
  out.SetMapFormat(EMITTER_MANIP::LongKey);

  out << EMITTER_MANIP::BeginMap;
  out << EMITTER_MANIP::Key << Anchor("key");
  out << EMITTER_MANIP::BeginSeq << "a"
      << "b" << EMITTER_MANIP::EndSeq;
  out << EMITTER_MANIP::Value << Anchor("value");
  out << EMITTER_MANIP::BeginSeq << "c"
      << "d" << EMITTER_MANIP::EndSeq;
  out << EMITTER_MANIP::Key << Alias("key") << EMITTER_MANIP::Value << Alias("value");
  out << EMITTER_MANIP::EndMap;

  ExpectEmit(R"(? &key
  - a
  - b
: &value
  - c
  - d
? *key
: *value)");
}

TEST_F(EmitterTest, GlobalLongKeyOnMap) {
  out.SetMapFormat(EMITTER_MANIP::LongKey);

  out << EMITTER_MANIP::BeginMap;
  out << EMITTER_MANIP::Key << Anchor("key");
  out << EMITTER_MANIP::BeginMap << "a"
      << "b" << EMITTER_MANIP::EndMap;
  out << EMITTER_MANIP::Value << Anchor("value");
  out << EMITTER_MANIP::BeginMap << "c"
      << "d" << EMITTER_MANIP::EndMap;
  out << EMITTER_MANIP::Key << Alias("key") << EMITTER_MANIP::Value << Alias("value");
  out << EMITTER_MANIP::EndMap;

  ExpectEmit(R"(? &key
  ? a
  : b
: &value
  ? c
  : d
? *key
: *value)");
}

TEST_F(EmitterTest, GlobalSettingStyleOnSeqNode) {
  Node n(Load(R"(foo:
  - 1
  - 2
  - 3
bar: aa)"));
  out.SetSeqFormat(EMITTER_MANIP::Flow);
  out << n;
  ExpectEmit(R"(foo: [1, 2, 3]
bar: aa)");
}

TEST_F(EmitterTest, GlobalSettingStyleOnMapNode) {
  Node n(Load(R"(-
  foo: a
  bar: b
- 2
- 3)"));
  out.SetMapFormat(EMITTER_MANIP::Flow);
  out << n;
  ExpectEmit(R"(- {foo: a, bar: b}
- 2
- 3)");
}

TEST_F(EmitterTest, ComplexGlobalSettings) {
  out << EMITTER_MANIP::BeginSeq;
  out << EMITTER_MANIP::Block;
  out << EMITTER_MANIP::BeginMap;
  out << EMITTER_MANIP::Key << "key 1" << EMITTER_MANIP::Value << "value 1";
  out << EMITTER_MANIP::Key << "key 2" << EMITTER_MANIP::Value;
  out.SetSeqFormat(EMITTER_MANIP::Flow);
  out << EMITTER_MANIP::BeginSeq << "a"
      << "b"
      << "c" << EMITTER_MANIP::EndSeq;
  out << EMITTER_MANIP::EndMap;
  out << EMITTER_MANIP::BeginMap;
  out << EMITTER_MANIP::Key << EMITTER_MANIP::BeginSeq << 1 << 2 << EMITTER_MANIP::EndSeq;
  out << EMITTER_MANIP::Value << EMITTER_MANIP::BeginMap << EMITTER_MANIP::Key << "a" << EMITTER_MANIP::Value << "b" << EMITTER_MANIP::EndMap;
  out << EMITTER_MANIP::EndMap;
  out << EMITTER_MANIP::EndSeq;

  ExpectEmit("- key 1: value 1\n  key 2: [a, b, c]\n- [1, 2]:\n    a: b");
}

TEST_F(EmitterTest, Null) {
  out << EMITTER_MANIP::BeginSeq;
  out << Null;
  out << EMITTER_MANIP::BeginMap;
  out << EMITTER_MANIP::Key << "null value" << EMITTER_MANIP::Value << Null;
  out << EMITTER_MANIP::Key << Null << EMITTER_MANIP::Value << "null key";
  out << EMITTER_MANIP::EndMap;
  out << EMITTER_MANIP::EndSeq;

  ExpectEmit("- ~\n- null value: ~\n  ~: null key");
}

TEST_F(EmitterTest, OutputCharset) {
  out << EMITTER_MANIP::BeginSeq;
  out.SetOutputCharset(EMITTER_MANIP::EmitNonAscii);
  out << "\x24 \xC2\xA2 \xE2\x82\xAC";
  out.SetOutputCharset(EMITTER_MANIP::EscapeNonAscii);
  out << "\x24 \xC2\xA2 \xE2\x82\xAC";
  out << EMITTER_MANIP::EndSeq;

  ExpectEmit("- \x24 \xC2\xA2 \xE2\x82\xAC\n- \"\x24 \\xa2 \\u20ac\"");
}

TEST_F(EmitterTest, EscapedUnicode) {
  out << EMITTER_MANIP::EscapeNonAscii << "\x24 \xC2\xA2 \xE2\x82\xAC \xF0\xA4\xAD\xA2";

  ExpectEmit("\"$ \\xa2 \\u20ac \\U00024b62\"");
}

TEST_F(EmitterTest, Unicode) {
  out << "\x24 \xC2\xA2 \xE2\x82\xAC \xF0\xA4\xAD\xA2";
  ExpectEmit("\x24 \xC2\xA2 \xE2\x82\xAC \xF0\xA4\xAD\xA2");
}

TEST_F(EmitterTest, DoubleQuotedUnicode) {
  out << EMITTER_MANIP::DoubleQuoted << "\x24 \xC2\xA2 \xE2\x82\xAC \xF0\xA4\xAD\xA2";
  ExpectEmit("\"\x24 \xC2\xA2 \xE2\x82\xAC \xF0\xA4\xAD\xA2\""); 
}

TEST_F(EmitterTest, EscapedJsonString) {
  out.SetStringFormat(EMITTER_MANIP::DoubleQuoted);
  out.SetOutputCharset(EMITTER_MANIP::EscapeAsJson);
  out << "\" \\ "
    "\x01 \x02 \x03 \x04 \x05 \x06 \x07 \x08 \x09 \x0A \x0B \x0C \x0D \x0E \x0F "
    "\x10 \x11 \x12 \x13 \x14 \x15 \x16 \x17 \x18 \x19 \x1A \x1B \x1C \x1D \x1E \x1F "
    "\x24 \xC2\xA2 \xE2\x82\xAC \xF0\xA4\xAD\xA2";

  ExpectEmit(R"("\" \\ \u0001 \u0002 \u0003 \u0004 \u0005 \u0006 \u0007 \b \t )"
             R"(\n \u000b \f \r \u000e \u000f \u0010 \u0011 \u0012 \u0013 )"
             R"(\u0014 \u0015 \u0016 \u0017 \u0018 \u0019 \u001a \u001b )"
             R"(\u001c \u001d \u001e \u001f )"
             "$ \xC2\xA2 \xE2\x82\xAC \xF0\xA4\xAD\xA2\"");
}

TEST_F(EmitterTest, EscapedCharacters) {
  out << EMITTER_MANIP::BeginSeq 
    << '\x00'
    << '\x0C'
    << '\x0D'
    << EMITTER_MANIP::EndSeq;

  ExpectEmit("- \"\\x00\"\n- \"\\f\"\n- \"\\r\"");
}

TEST_F(EmitterTest, CharactersEscapedAsJson) {
  out.SetOutputCharset(EMITTER_MANIP::EscapeAsJson);
  out << EMITTER_MANIP::BeginSeq
    << '\x00'
    << '\x0C'
    << '\x0D'
    << EMITTER_MANIP::EndSeq;

  ExpectEmit("- \"\\u0000\"\n- \"\\f\"\n- \"\\r\"");
}

TEST_F(EmitterTest, DoubleQuotedString) {
  out << EMITTER_MANIP::DoubleQuoted << "\" \\ \n \t \r \b \x15 \xEF\xBB\xBF \x24";
  ExpectEmit("\"\\\" \\\\ \\n \\t \\r \\b \\x15 \\ufeff $\"");
}

struct Foo {
  Foo() : x(0) {}
  Foo(int x_, const std::string& bar_) : x(x_), bar(bar_) {}

  int x;
  std::string bar;
};

Emitter& operator<<(Emitter& out, const Foo& foo) {
  out << EMITTER_MANIP::BeginMap;
  out << EMITTER_MANIP::Key << "x" << EMITTER_MANIP::Value << foo.x;
  out << EMITTER_MANIP::Key << "bar" << EMITTER_MANIP::Value << foo.bar;
  out << EMITTER_MANIP::EndMap;
  return out;
}

TEST_F(EmitterTest, UserType) {
  out << EMITTER_MANIP::BeginSeq;
  out << Foo(5, "hello");
  out << Foo(3, "goodbye");
  out << EMITTER_MANIP::EndSeq;

  ExpectEmit("- x: 5\n  bar: hello\n- x: 3\n  bar: goodbye");
}

TEST_F(EmitterTest, UserTypeInContainer) {
  std::vector<Foo> fv;
  fv.push_back(Foo(5, "hello"));
  fv.push_back(Foo(3, "goodbye"));
  out << fv;

  ExpectEmit("- x: 5\n  bar: hello\n- x: 3\n  bar: goodbye");
}

template <typename T>
Emitter& operator<<(Emitter& out, const T* v) {
  if (v)
    out << *v;
  else
    out << Null;
  return out;
}

TEST_F(EmitterTest, PointerToInt) {
  int foo = 5;
  int* bar = &foo;
  int* baz = 0;
  out << EMITTER_MANIP::BeginSeq;
  out << bar << baz;
  out << EMITTER_MANIP::EndSeq;

  ExpectEmit("- 5\n- ~");
}

TEST_F(EmitterTest, PointerToUserType) {
  Foo foo(5, "hello");
  Foo* bar = &foo;
  Foo* baz = 0;
  out << EMITTER_MANIP::BeginSeq;
  out << bar << baz;
  out << EMITTER_MANIP::EndSeq;

  ExpectEmit("- x: 5\n  bar: hello\n- ~");
}

TEST_F(EmitterTest, NewlineAtEnd) {
  out << "Hello" << EMITTER_MANIP::Newline << EMITTER_MANIP::Newline;
  ExpectEmit("Hello\n\n");
}

TEST_F(EmitterTest, NewlineInBlockSequence) {
  out << EMITTER_MANIP::BeginSeq;
  out << "a" << EMITTER_MANIP::Newline << "b"
      << "c" << EMITTER_MANIP::Newline << "d";
  out << EMITTER_MANIP::EndSeq;
  ExpectEmit("- a\n\n- b\n- c\n\n- d");
}

TEST_F(EmitterTest, NewlineInFlowSequence) {
  out << EMITTER_MANIP::Flow << EMITTER_MANIP::BeginSeq;
  out << "a" << EMITTER_MANIP::Newline << "b"
      << "c" << EMITTER_MANIP::Newline << "d";
  out << EMITTER_MANIP::EndSeq;
  ExpectEmit("[a,\nb, c,\nd]");
}

TEST_F(EmitterTest, NewlineInBlockMap) {
  out << EMITTER_MANIP::BeginMap;
  out << EMITTER_MANIP::Key << "a" << EMITTER_MANIP::Value << "foo" << EMITTER_MANIP::Newline;
  out << EMITTER_MANIP::Key << "b" << EMITTER_MANIP::Newline << EMITTER_MANIP::Value << "bar";
  out << EMITTER_MANIP::LongKey << EMITTER_MANIP::Key << "c" << EMITTER_MANIP::Newline << EMITTER_MANIP::Value << "car";
  out << EMITTER_MANIP::EndMap;
  ExpectEmit("a: foo\nb:\n  bar\n? c\n\n: car");
}

TEST_F(EmitterTest, NewlineInFlowMap) {
  out << EMITTER_MANIP::Flow << EMITTER_MANIP::BeginMap;
  out << EMITTER_MANIP::Key << "a" << EMITTER_MANIP::Value << "foo" << EMITTER_MANIP::Newline;
  out << EMITTER_MANIP::Key << "b" << EMITTER_MANIP::Value << "bar";
  out << EMITTER_MANIP::EndMap;
  ExpectEmit("{a: foo,\nb: bar}");
}

TEST_F(EmitterTest, LotsOfNewlines) {
  out << EMITTER_MANIP::BeginSeq;
  out << "a" << EMITTER_MANIP::Newline;
  out << EMITTER_MANIP::BeginSeq;
  out << "b"
      << "c" << EMITTER_MANIP::Newline;
  out << EMITTER_MANIP::EndSeq;
  out << EMITTER_MANIP::Newline;
  out << EMITTER_MANIP::BeginMap;
  out << EMITTER_MANIP::Newline << EMITTER_MANIP::Key << "d" << EMITTER_MANIP::Value << EMITTER_MANIP::Newline << "e";
  out << EMITTER_MANIP::LongKey << EMITTER_MANIP::Key << "f" << EMITTER_MANIP::Newline << EMITTER_MANIP::Value << "foo";
  out << EMITTER_MANIP::EndMap;
  out << EMITTER_MANIP::EndSeq;
  ExpectEmit("- a\n\n-\n  - b\n  - c\n\n\n-\n  d:\n    e\n  ? f\n\n  : foo");
}

TEST_F(EmitterTest, Binary) {
  out << Binary(reinterpret_cast<const unsigned char*>("Hello, World!"), 13);
  ExpectEmit("!!binary \"SGVsbG8sIFdvcmxkIQ==\"");
}

TEST_F(EmitterTest, LongBinary) {
  out << Binary(
      reinterpret_cast<const unsigned char*>(
          "Man is distinguished, not only by his reason, but by this "
          "singular passion from other animals, which is a lust of the "
          "mind, that by a perseverance of delight in the continued and "
          "indefatigable generation of knowledge, exceeds the short "
          "vehemence of any carnal pleasure.\n"),
      270);
  ExpectEmit(
      "!!binary "
      "\"TWFuIGlzIGRpc3Rpbmd1aXNoZWQsIG5vdCBvbmx5IGJ5IGhpcyByZWFzb24sIGJ1dCBieS"
      "B0aGlzIHNpbmd1bGFyIHBhc3Npb24gZnJvbSBvdGhlciBhbmltYWxzLCB3aGljaCBpcyBhIG"
      "x1c3Qgb2YgdGhlIG1pbmQsIHRoYXQgYnkgYSBwZXJzZXZlcmFuY2Ugb2YgZGVsaWdodCBpbi"
      "B0aGUgY29udGludWVkIGFuZCBpbmRlZmF0aWdhYmxlIGdlbmVyYXRpb24gb2Yga25vd2xlZG"
      "dlLCBleGNlZWRzIHRoZSBzaG9ydCB2ZWhlbWVuY2Ugb2YgYW55IGNhcm5hbCBwbGVhc3VyZS"
      "4K\"");
}

TEST_F(EmitterTest, EmptyBinary) {
  out << Binary(reinterpret_cast<const unsigned char*>(""), 0);
  ExpectEmit("!!binary \"\"");
}

TEST_F(EmitterTest, ColonAtEndOfScalar) {
  out << "a:";
  ExpectEmit("\"a:\"");
}

TEST_F(EmitterTest, ColonAsScalar) {
  out << EMITTER_MANIP::BeginMap;
  out << EMITTER_MANIP::Key << "apple" << EMITTER_MANIP::Value << ":";
  out << EMITTER_MANIP::Key << "banana" << EMITTER_MANIP::Value << ":";
  out << EMITTER_MANIP::EndMap;
  ExpectEmit("apple: \":\"\nbanana: \":\"");
}

TEST_F(EmitterTest, ColonAtEndOfScalarInFlow) {
  out << EMITTER_MANIP::Flow << EMITTER_MANIP::BeginMap << EMITTER_MANIP::Key << "C:" << EMITTER_MANIP::Value << "C:" << EMITTER_MANIP::EndMap;
  ExpectEmit("{\"C:\": \"C:\"}");
}

TEST_F(EmitterTest, GlobalBoolFormatting) {
  out << EMITTER_MANIP::BeginSeq;
  out.SetBoolFormat(EMITTER_MANIP::UpperCase);
  out.SetBoolFormat(EMITTER_MANIP::YesNoBool);
  out << true;
  out << false;
  out.SetBoolFormat(EMITTER_MANIP::TrueFalseBool);
  out << true;
  out << false;
  out.SetBoolFormat(EMITTER_MANIP::OnOffBool);
  out << true;
  out << false;
  out.SetBoolFormat(EMITTER_MANIP::LowerCase);
  out.SetBoolFormat(EMITTER_MANIP::YesNoBool);
  out << true;
  out << false;
  out.SetBoolFormat(EMITTER_MANIP::TrueFalseBool);
  out << true;
  out << false;
  out.SetBoolFormat(EMITTER_MANIP::OnOffBool);
  out << true;
  out << false;
  out.SetBoolFormat(EMITTER_MANIP::CamelCase);
  out.SetBoolFormat(EMITTER_MANIP::YesNoBool);
  out << true;
  out << false;
  out.SetBoolFormat(EMITTER_MANIP::TrueFalseBool);
  out << true;
  out << false;
  out.SetBoolFormat(EMITTER_MANIP::OnOffBool);
  out << true;
  out << false;
  out.SetBoolFormat(EMITTER_MANIP::ShortBool);
  out.SetBoolFormat(EMITTER_MANIP::UpperCase);
  out.SetBoolFormat(EMITTER_MANIP::YesNoBool);
  out << true;
  out << false;
  out.SetBoolFormat(EMITTER_MANIP::TrueFalseBool);
  out << true;
  out << false;
  out.SetBoolFormat(EMITTER_MANIP::OnOffBool);
  out << true;
  out << false;
  out << EMITTER_MANIP::EndSeq;
  ExpectEmit(
      "- YES\n- NO\n- TRUE\n- FALSE\n- ON\n- OFF\n"
      "- yes\n- no\n- true\n- false\n- on\n- off\n"
      "- Yes\n- No\n- True\n- False\n- On\n- Off\n"
      "- Y\n- N\n- Y\n- N\n- Y\n- N");
}

TEST_F(EmitterTest, BoolFormatting) {
  out << EMITTER_MANIP::BeginSeq;
  out << EMITTER_MANIP::TrueFalseBool << EMITTER_MANIP::UpperCase << true;
  out << EMITTER_MANIP::TrueFalseBool << EMITTER_MANIP::CamelCase << true;
  out << EMITTER_MANIP::TrueFalseBool << EMITTER_MANIP::LowerCase << true;
  out << EMITTER_MANIP::TrueFalseBool << EMITTER_MANIP::UpperCase << false;
  out << EMITTER_MANIP::TrueFalseBool << EMITTER_MANIP::CamelCase << false;
  out << EMITTER_MANIP::TrueFalseBool << EMITTER_MANIP::LowerCase << false;
  out << EMITTER_MANIP::YesNoBool << EMITTER_MANIP::UpperCase << true;
  out << EMITTER_MANIP::YesNoBool << EMITTER_MANIP::CamelCase << true;
  out << EMITTER_MANIP::YesNoBool << EMITTER_MANIP::LowerCase << true;
  out << EMITTER_MANIP::YesNoBool << EMITTER_MANIP::UpperCase << false;
  out << EMITTER_MANIP::YesNoBool << EMITTER_MANIP::CamelCase << false;
  out << EMITTER_MANIP::YesNoBool << EMITTER_MANIP::LowerCase << false;
  out << EMITTER_MANIP::OnOffBool << EMITTER_MANIP::UpperCase << true;
  out << EMITTER_MANIP::OnOffBool << EMITTER_MANIP::CamelCase << true;
  out << EMITTER_MANIP::OnOffBool << EMITTER_MANIP::LowerCase << true;
  out << EMITTER_MANIP::OnOffBool << EMITTER_MANIP::UpperCase << false;
  out << EMITTER_MANIP::OnOffBool << EMITTER_MANIP::CamelCase << false;
  out << EMITTER_MANIP::OnOffBool << EMITTER_MANIP::LowerCase << false;
  out << EMITTER_MANIP::ShortBool << EMITTER_MANIP::UpperCase << true;
  out << EMITTER_MANIP::ShortBool << EMITTER_MANIP::CamelCase << true;
  out << EMITTER_MANIP::ShortBool << EMITTER_MANIP::LowerCase << true;
  out << EMITTER_MANIP::ShortBool << EMITTER_MANIP::UpperCase << false;
  out << EMITTER_MANIP::ShortBool << EMITTER_MANIP::CamelCase << false;
  out << EMITTER_MANIP::ShortBool << EMITTER_MANIP::LowerCase << false;
  out << EMITTER_MANIP::EndSeq;
  ExpectEmit(
      "- TRUE\n- True\n- true\n- FALSE\n- False\n- false\n"
      "- YES\n- Yes\n- yes\n- NO\n- No\n- no\n"
      "- ON\n- On\n- on\n- OFF\n- Off\n- off\n"
      "- Y\n- Y\n- y\n- N\n- N\n- n");
}

TEST_F(EmitterTest, GlobalNullFormatting) {
  out << EMITTER_MANIP::Flow << EMITTER_MANIP::BeginSeq;
  out.SetNullFormat(EMITTER_MANIP::LowerNull);
  out << Null;
  out.SetNullFormat(EMITTER_MANIP::UpperNull);
  out << Null;
  out.SetNullFormat(EMITTER_MANIP::CamelNull);
  out << Null;
  out.SetNullFormat(EMITTER_MANIP::TildeNull);
  out << Null;
  out << EMITTER_MANIP::EndSeq;
  ExpectEmit("[null, NULL, Null, ~]");
}

TEST_F(EmitterTest, NullFormatting) {
  out << EMITTER_MANIP::Flow << EMITTER_MANIP::BeginSeq;
  out << EMITTER_MANIP::LowerNull << Null;
  out << EMITTER_MANIP::UpperNull << Null;
  out << EMITTER_MANIP::CamelNull << Null;
  out << EMITTER_MANIP::TildeNull << Null;
  out << EMITTER_MANIP::EndSeq;
  ExpectEmit("[null, NULL, Null, ~]");
}

TEST_F(EmitterTest, NullFormattingOnNode) {
  Node n(Load("null"));
  out << EMITTER_MANIP::Flow << EMITTER_MANIP::BeginSeq;
  out.SetNullFormat(EMITTER_MANIP::LowerNull);
  out << n;
  out.SetNullFormat(EMITTER_MANIP::UpperNull);
  out << n;
  out.SetNullFormat(EMITTER_MANIP::CamelNull);
  out << n;
  out.SetNullFormat(EMITTER_MANIP::TildeNull);
  out << n;
  out << EMITTER_MANIP::EndSeq;
  ExpectEmit("[null, NULL, Null, ~]");
}

// TODO: Fix this test.
// TEST_F(EmitterTest, DocStartAndEnd) {
//  out << EMITTER_MANIP::BeginDoc;
//  out << EMITTER_MANIP::BeginSeq << 1 << 2 << 3 << EMITTER_MANIP::EndSeq;
//  out << EMITTER_MANIP::BeginDoc;
//  out << "Hi there!";
//  out << EMITTER_MANIP::EndDoc;
//  out << EMITTER_MANIP::EndDoc;
//  out << EMITTER_MANIP::EndDoc;
//  out << EMITTER_MANIP::BeginDoc;
//  out << VerbatimTag("foo") << "bar";
//  ExpectEmit(
//      "---\n- 1\n- 2\n- 3\n---\nHi there!\n...\n...\n...\n---\n!<foo> bar");
//}

TEST_F(EmitterTest, ImplicitDocStart) {
  out << "Hi";
  out << "Bye";
  out << "Oops";
  ExpectEmit("Hi\n---\nBye\n---\nOops");
}

TEST_F(EmitterTest, EmptyString) {
  out << EMITTER_MANIP::BeginMap;
  out << EMITTER_MANIP::Key << "key" << EMITTER_MANIP::Value << "";
  out << EMITTER_MANIP::EndMap;
  ExpectEmit("key: \"\"");
}

TEST_F(EmitterTest, SingleChar) {
  out << EMITTER_MANIP::BeginSeq;
  out << 'a';
  out << ':';
  out << (char)0x10;
  out << '\n';
  out << ' ';
  out << '\t';
  out << EMITTER_MANIP::EndSeq;
  ExpectEmit("- a\n- \":\"\n- \"\\x10\"\n- \"\\n\"\n- \" \"\n- \"\\t\"");
}

TEST_F(EmitterTest, DefaultPrecision) {
  out << EMITTER_MANIP::BeginSeq;
  out << 1.3125f;
  out << 1.23455810546875;
  out << EMITTER_MANIP::EndSeq;
  ExpectEmit("- 1.3125\n- 1.23455810546875");
}

TEST_F(EmitterTest, SetPrecision) {
  out << EMITTER_MANIP::BeginSeq;
  out << FloatPrecision(3) << 1.3125f;
  out << DoublePrecision(6) << 1.23455810546875;
  out << EMITTER_MANIP::EndSeq;
  ExpectEmit("- 1.31\n- 1.23456");
}

TEST_F(EmitterTest, DashInBlockContext) {
  out << EMITTER_MANIP::BeginMap;
  out << EMITTER_MANIP::Key << "key" << EMITTER_MANIP::Value << "-";
  out << EMITTER_MANIP::EndMap;
  ExpectEmit("key: \"-\"");
}

TEST_F(EmitterTest, HexAndOct) {
  out << EMITTER_MANIP::Flow << EMITTER_MANIP::BeginSeq;
  out << 31;
  out << EMITTER_MANIP::Hex << 31;
  out << EMITTER_MANIP::Oct << 31;
  out << EMITTER_MANIP::EndSeq;
  ExpectEmit("[31, 0x1f, 037]");
}

TEST_F(EmitterTest, CompactMapWithNewline) {
  out << Comment("Characteristics");
  out << EMITTER_MANIP::BeginSeq;
  out << EMITTER_MANIP::BeginMap;
  out << EMITTER_MANIP::Key << "color" << EMITTER_MANIP::Value << "blue";
  out << EMITTER_MANIP::Key << "height" << EMITTER_MANIP::Value << 120;
  out << EMITTER_MANIP::EndMap;
  out << EMITTER_MANIP::Newline << EMITTER_MANIP::Newline;
  out << Comment("Skills");
  out << EMITTER_MANIP::BeginMap;
  out << EMITTER_MANIP::Key << "attack" << EMITTER_MANIP::Value << 23;
  out << EMITTER_MANIP::Key << "intelligence" << EMITTER_MANIP::Value << 56;
  out << EMITTER_MANIP::EndMap;
  out << EMITTER_MANIP::EndSeq;

  ExpectEmit(
      "# Characteristics\n"
      "- color: blue\n"
      "  height: 120\n"
      "\n"
      "# Skills\n"
      "- attack: 23\n"
      "  intelligence: 56");
}

TEST_F(EmitterTest, ForceSingleQuotedToDouble) {
  out << EMITTER_MANIP::SingleQuoted << "Hello\nWorld";

  ExpectEmit("\"Hello\\nWorld\"");
}

TEST_F(EmitterTest, QuoteNull) {
  out << "null";

  ExpectEmit("\"null\"");
}

TEST_F(EmitterTest, ValueOfDoubleQuote) {
  out << EMITTER_MANIP::BeginMap;
  out << EMITTER_MANIP::Key << "foo" << EMITTER_MANIP::Value << '"';
  out << EMITTER_MANIP::EndMap;

  ExpectEmit("foo: \"\\\"\"");
}

TEST_F(EmitterTest, ValueOfBackslash) {
  out << EMITTER_MANIP::BeginMap;
  out << EMITTER_MANIP::Key << "foo" << EMITTER_MANIP::Value << '\\';
  out << EMITTER_MANIP::EndMap;

  ExpectEmit("foo: \"\\\\\"");
}

TEST_F(EmitterTest, Infinity) {
  out << EMITTER_MANIP::BeginMap;
  out << EMITTER_MANIP::Key << "foo" << EMITTER_MANIP::Value
      << std::numeric_limits<float>::infinity();
  out << EMITTER_MANIP::Key << "bar" << EMITTER_MANIP::Value
      << std::numeric_limits<double>::infinity();
  out << EMITTER_MANIP::EndMap;

  ExpectEmit(
	  "foo: .inf\n"
	  "bar: .inf");
}

TEST_F(EmitterTest, NegInfinity) {
  out << EMITTER_MANIP::BeginMap;
  out << EMITTER_MANIP::Key << "foo" << EMITTER_MANIP::Value
      << -std::numeric_limits<float>::infinity();
  out << EMITTER_MANIP::Key << "bar" << EMITTER_MANIP::Value
      << -std::numeric_limits<double>::infinity();
  out << EMITTER_MANIP::EndMap;

  ExpectEmit(
	  "foo: -.inf\n"
	  "bar: -.inf");
}

TEST_F(EmitterTest, NaN) {
  out << EMITTER_MANIP::BeginMap;
  out << EMITTER_MANIP::Key << "foo" << EMITTER_MANIP::Value
      << std::numeric_limits<float>::quiet_NaN();
  out << EMITTER_MANIP::Key << "bar" << EMITTER_MANIP::Value
      << std::numeric_limits<double>::quiet_NaN();
  out << EMITTER_MANIP::EndMap;

  ExpectEmit(
	  "foo: .nan\n"
	  "bar: .nan");
}

TEST_F(EmitterTest, ComplexFlowSeqEmbeddingAMapWithNewLine) { 
  out << EMITTER_MANIP::BeginMap;

  out << EMITTER_MANIP::Key << "NodeA" << EMITTER_MANIP::Value << EMITTER_MANIP::BeginMap;
  out << EMITTER_MANIP::Key << "k" << EMITTER_MANIP::Value << EMITTER_MANIP::Flow << EMITTER_MANIP::BeginSeq;
  out << EMITTER_MANIP::BeginMap << EMITTER_MANIP::Key << "i" << EMITTER_MANIP::Value << 0 << EMITTER_MANIP::EndMap
      << EMITTER_MANIP::Newline;
  out << EMITTER_MANIP::BeginMap << EMITTER_MANIP::Key << "i" << EMITTER_MANIP::Value << 1 << EMITTER_MANIP::EndMap
      << EMITTER_MANIP::Newline;
  out << EMITTER_MANIP::EndSeq;
  out << EMITTER_MANIP::EndMap;

  out << EMITTER_MANIP::Key << "NodeB" << EMITTER_MANIP::Value << EMITTER_MANIP::BeginMap;
  out << EMITTER_MANIP::Key << "k" << EMITTER_MANIP::Value << EMITTER_MANIP::Flow << EMITTER_MANIP::BeginSeq;
  out << EMITTER_MANIP::BeginMap << EMITTER_MANIP::Key << "i" << EMITTER_MANIP::Value << 0 << EMITTER_MANIP::EndMap
      << EMITTER_MANIP::Newline;
  out << EMITTER_MANIP::BeginMap << EMITTER_MANIP::Key << "i" << EMITTER_MANIP::Value << 1 << EMITTER_MANIP::EndMap
      << EMITTER_MANIP::Newline;
  out << EMITTER_MANIP::EndSeq;
  out << EMITTER_MANIP::EndMap;

  out << EMITTER_MANIP::EndMap;

  ExpectEmit(R"(NodeA:
  k: [{i: 0},
    {i: 1},
    ]
NodeB:
  k: [{i: 0},
    {i: 1},
    ])");
}

TEST_F(EmitterTest, ComplexFlowSeqEmbeddingAMapWithNewLineUsingAliases) {
  out << EMITTER_MANIP::BeginMap;

  out << EMITTER_MANIP::Key << "Node" << Anchor("Node") << EMITTER_MANIP::Value << EMITTER_MANIP::BeginMap;
  out << EMITTER_MANIP::Key << "k" << EMITTER_MANIP::Value << EMITTER_MANIP::Flow << EMITTER_MANIP::BeginSeq;
  out << EMITTER_MANIP::BeginMap << EMITTER_MANIP::Key << "i" << EMITTER_MANIP::Value << 0 << EMITTER_MANIP::EndMap;
  out << EMITTER_MANIP::Newline;
  out << EMITTER_MANIP::BeginMap << EMITTER_MANIP::Key << "i" << EMITTER_MANIP::Value << 1 << EMITTER_MANIP::EndMap;
  out << EMITTER_MANIP::Newline;
  out << EMITTER_MANIP::EndSeq << EMITTER_MANIP::EndMap;

  out << EMITTER_MANIP::Key << "NodeA" << Alias("Node");
  out << EMITTER_MANIP::Key << "NodeB" << Alias("Node");

  out << EMITTER_MANIP::EndMap;

  ExpectEmit(R"(Node: &Node
  k: [{i: 0},
    {i: 1},
    ]
NodeA: *Node
NodeB: *Node)");
}

TEST_F(EmitterTest, ComplexFlowSeqEmbeddingAMapUsingAliases) {
  out << EMITTER_MANIP::BeginMap;

  out << EMITTER_MANIP::Key << "Node" << Anchor("Node") << EMITTER_MANIP::Value << EMITTER_MANIP::BeginMap;
  out << EMITTER_MANIP::Key << "k" << EMITTER_MANIP::Value << EMITTER_MANIP::Flow << EMITTER_MANIP::BeginSeq;
  out << EMITTER_MANIP::BeginMap << EMITTER_MANIP::Key << "i" << EMITTER_MANIP::Value << 0 << EMITTER_MANIP::EndMap;
  out << EMITTER_MANIP::BeginMap << EMITTER_MANIP::Key << "i" << EMITTER_MANIP::Value << 1 << EMITTER_MANIP::EndMap;
  out << EMITTER_MANIP::EndSeq << EMITTER_MANIP::EndMap;

  out << EMITTER_MANIP::Key << "NodeA" << Alias("Node");
  out << EMITTER_MANIP::Key << "NodeB" << Alias("Node");

  out << EMITTER_MANIP::EndMap;

  ExpectEmit(R"(Node: &Node
  k: [{i: 0}, {i: 1}]
NodeA: *Node
NodeB: *Node)");
}

TEST_F(EmitterTest, ComplexFlowSeqEmbeddingAMapWithNewLineUsingAliases2) {
  out << EMITTER_MANIP::BeginMap;

  out << EMITTER_MANIP::Key << "Seq" << Anchor("Seq") << EMITTER_MANIP::Flow << EMITTER_MANIP::BeginSeq;
  out << EMITTER_MANIP::BeginMap << EMITTER_MANIP::Key << "i" << EMITTER_MANIP::Value << 0 << EMITTER_MANIP::EndMap;
  out << EMITTER_MANIP::Newline;
  out << EMITTER_MANIP::BeginMap << EMITTER_MANIP::Key << "i" << EMITTER_MANIP::Value << 1 << EMITTER_MANIP::EndMap;
  out << EMITTER_MANIP::Newline;
  out << EMITTER_MANIP::EndSeq;

  out << EMITTER_MANIP::Key << "NodeA" << EMITTER_MANIP::Value << EMITTER_MANIP::BeginMap;
  out << EMITTER_MANIP::Key << "k" << EMITTER_MANIP::Value << Alias("Seq") << EMITTER_MANIP::EndMap;
  out << EMITTER_MANIP::Key << "NodeB" << EMITTER_MANIP::Value << EMITTER_MANIP::BeginMap;
  out << EMITTER_MANIP::Key << "k" << EMITTER_MANIP::Value << Alias("Seq") << EMITTER_MANIP::EndMap;

  out << EMITTER_MANIP::EndMap;

  ExpectEmit(R"(Seq: &Seq [{i: 0},
  {i: 1},
  ]
NodeA:
  k: *Seq
NodeB:
  k: *Seq)");
}

TEST_F(EmitterTest, ComplexFlowSeqEmbeddingAMapUsingAliases2) {
  out << EMITTER_MANIP::BeginMap;

  out << EMITTER_MANIP::Key << "Seq" << Anchor("Seq") << EMITTER_MANIP::Value << EMITTER_MANIP::Flow << EMITTER_MANIP::BeginSeq;
  out << EMITTER_MANIP::BeginMap << EMITTER_MANIP::Key << "i" << EMITTER_MANIP::Value << 0 << EMITTER_MANIP::EndMap;
  out << EMITTER_MANIP::BeginMap << EMITTER_MANIP::Key << "i" << EMITTER_MANIP::Value << 1 << EMITTER_MANIP::EndMap;
  out << EMITTER_MANIP::EndSeq;

  out << EMITTER_MANIP::Key << "NodeA" << EMITTER_MANIP::Value << EMITTER_MANIP::BeginMap;
  out << EMITTER_MANIP::Key << "k" << EMITTER_MANIP::Value << Alias("Seq") << EMITTER_MANIP::EndMap;
  out << EMITTER_MANIP::Key << "NodeB" << EMITTER_MANIP::Value << EMITTER_MANIP::BeginMap;
  out << EMITTER_MANIP::Key << "k" << EMITTER_MANIP::Value << Alias("Seq") << EMITTER_MANIP::EndMap;

  out << EMITTER_MANIP::EndMap;

  ExpectEmit(R"(Seq: &Seq [{i: 0}, {i: 1}]
NodeA:
  k: *Seq
NodeB:
  k: *Seq)");
}

TEST_F(EmitterTest, ComplexFlowSeqEmbeddingAMapWithNewLineUsingAliases3) {
  out << EMITTER_MANIP::BeginMap;

  out << EMITTER_MANIP::Key << "Keys" << EMITTER_MANIP::Value << EMITTER_MANIP::Flow << EMITTER_MANIP::BeginSeq;
  out << Anchor("k0") << EMITTER_MANIP::BeginMap << EMITTER_MANIP::Key << "i" << EMITTER_MANIP::Value << 0 << EMITTER_MANIP::EndMap
      << EMITTER_MANIP::Newline;
  out << Anchor("k1") << EMITTER_MANIP::BeginMap << EMITTER_MANIP::Key << "i" << EMITTER_MANIP::Value << 1 << EMITTER_MANIP::EndMap
      << EMITTER_MANIP::Newline;
  out << EMITTER_MANIP::EndSeq;

  out << EMITTER_MANIP::Key << "NodeA" << EMITTER_MANIP::Value << EMITTER_MANIP::BeginMap;
  out << EMITTER_MANIP::Key << "k" << EMITTER_MANIP::Value << EMITTER_MANIP::Flow << EMITTER_MANIP::BeginSeq;
  out << Alias("k0") << EMITTER_MANIP::Newline << Alias("k1") << EMITTER_MANIP::Newline;
  out << EMITTER_MANIP::EndSeq << EMITTER_MANIP::EndMap;

  out << EMITTER_MANIP::Key << "NodeB" << EMITTER_MANIP::Value << EMITTER_MANIP::BeginMap;
  out << EMITTER_MANIP::Key << "k" << EMITTER_MANIP::Value << EMITTER_MANIP::Flow << EMITTER_MANIP::BeginSeq;
  out << Alias("k0") << EMITTER_MANIP::Newline << Alias("k1") << EMITTER_MANIP::Newline;
  out << EMITTER_MANIP::EndSeq << EMITTER_MANIP::EndMap;

  out << EMITTER_MANIP::EndMap;

  ExpectEmit(R"(Keys: [&k0 {i: 0},
&k1 {i: 1},
  ]
NodeA:
  k: [*k0,
  *k1,
    ]
NodeB:
  k: [*k0,
  *k1,
    ])");
}

TEST_F(EmitterTest, ComplexFlowSeqEmbeddingAMapUsingAliases3a) {
  out << EMITTER_MANIP::BeginMap;

  out << EMITTER_MANIP::Key << "Keys" << EMITTER_MANIP::Value << EMITTER_MANIP::BeginSeq;
  out << Anchor("k0") << EMITTER_MANIP::BeginMap << EMITTER_MANIP::Key << "i" << EMITTER_MANIP::Value << 0 << EMITTER_MANIP::EndMap;
  out << Anchor("k1") << EMITTER_MANIP::BeginMap << EMITTER_MANIP::Key << "i" << EMITTER_MANIP::Value << 1 << EMITTER_MANIP::EndMap;
  out << EMITTER_MANIP::EndSeq;

  out << EMITTER_MANIP::Key << "NodeA" << EMITTER_MANIP::Value << EMITTER_MANIP::BeginMap;
  out << EMITTER_MANIP::Key << "k" << EMITTER_MANIP::Value << EMITTER_MANIP::Flow << EMITTER_MANIP::BeginSeq;
  out << Alias("k0") << Alias("k1");
  out << EMITTER_MANIP::EndSeq << EMITTER_MANIP::EndMap;

  out << EMITTER_MANIP::Key << "NodeB" << EMITTER_MANIP::Value << EMITTER_MANIP::BeginMap;
  out << EMITTER_MANIP::Key << "k" << EMITTER_MANIP::Value << EMITTER_MANIP::Flow << EMITTER_MANIP::BeginSeq;
  out << Alias("k0") << Alias("k1");
  out << EMITTER_MANIP::EndSeq << EMITTER_MANIP::EndMap;

  out << EMITTER_MANIP::EndMap;

  ExpectEmit(R"(Keys:
  - &k0
    i: 0
  - &k1
    i: 1
NodeA:
  k: [*k0, *k1]
NodeB:
  k: [*k0, *k1])");
}

TEST_F(EmitterTest, ComplexFlowSeqEmbeddingAMapUsingAliases3b) {
  out << EMITTER_MANIP::BeginMap;

  out << EMITTER_MANIP::Key << "Keys" << EMITTER_MANIP::Value << EMITTER_MANIP::Flow << EMITTER_MANIP::BeginSeq;
  out << Anchor("k0") << EMITTER_MANIP::BeginMap << EMITTER_MANIP::Key << "i" << EMITTER_MANIP::Value << 0 << EMITTER_MANIP::EndMap;
  out << Anchor("k1") << EMITTER_MANIP::BeginMap << EMITTER_MANIP::Key << "i" << EMITTER_MANIP::Value << 1 << EMITTER_MANIP::EndMap;
  out << EMITTER_MANIP::EndSeq;

  out << EMITTER_MANIP::Key << "NodeA" << EMITTER_MANIP::Value << EMITTER_MANIP::BeginMap;
  out << EMITTER_MANIP::Key << "k" << EMITTER_MANIP::Value << EMITTER_MANIP::Flow << EMITTER_MANIP::BeginSeq;
  out << Alias("k0") << Alias("k1");
  out << EMITTER_MANIP::EndSeq << EMITTER_MANIP::EndMap;

  out << EMITTER_MANIP::Key << "NodeB" << EMITTER_MANIP::Value << EMITTER_MANIP::BeginMap;
  out << EMITTER_MANIP::Key << "k" << EMITTER_MANIP::Value << EMITTER_MANIP::Flow << EMITTER_MANIP::BeginSeq;
  out << Alias("k0") << Alias("k1");
  out << EMITTER_MANIP::EndSeq << EMITTER_MANIP::EndMap;

  out << EMITTER_MANIP::EndMap;

  ExpectEmit(R"(Keys: [&k0 {i: 0}, &k1 {i: 1}]
NodeA:
  k: [*k0, *k1]
NodeB:
  k: [*k0, *k1])");
}

class EmitterErrorTest : public ::testing::Test {
 protected:
  void ExpectEmitError(const std::string& expectedError) {
    ASSERT_FALSE(out.good()) << "Emitter cleanly produced: " << out.c_str();
    EXPECT_EQ(expectedError, out.GetLastError());
  }

  Emitter out;
};

TEST_F(EmitterErrorTest, BadLocalTag) {
  out << LocalTag("e!far") << "bar";

  ExpectEmitError("invalid tag");
}

TEST_F(EmitterErrorTest, BadTagAndTag) {
  out << VerbatimTag("!far") << VerbatimTag("!foo") << "bar";
  ExpectEmitError(ErrorMsg::INVALID_TAG);
}

TEST_F(EmitterErrorTest, BadAnchorAndAnchor) {
  out << Anchor("far") << Anchor("foo") << "bar";
  ExpectEmitError(ErrorMsg::INVALID_ANCHOR);
}

TEST_F(EmitterErrorTest, BadEmptyAnchorOnGroup) {
  out << EMITTER_MANIP::BeginSeq << "bar" << Anchor("foo") << EMITTER_MANIP::EndSeq;
  ExpectEmitError(ErrorMsg::INVALID_ANCHOR);
}

TEST_F(EmitterErrorTest, BadEmptyTagOnGroup) {
  out << EMITTER_MANIP::BeginSeq << "bar" << VerbatimTag("!foo") << EMITTER_MANIP::EndSeq;
  ExpectEmitError(ErrorMsg::INVALID_TAG);
}

TEST_F(EmitterErrorTest, ExtraEndSeq) {
  out << EMITTER_MANIP::BeginSeq;
  out << "Hello";
  out << "World";
  out << EMITTER_MANIP::EndSeq;
  out << EMITTER_MANIP::EndSeq;

  ExpectEmitError(ErrorMsg::UNEXPECTED_END_SEQ);
}

TEST_F(EmitterErrorTest, ExtraEndMap) {
  out << EMITTER_MANIP::BeginMap;
  out << EMITTER_MANIP::Key << "Hello" << EMITTER_MANIP::Value << "World";
  out << EMITTER_MANIP::EndMap;
  out << EMITTER_MANIP::EndMap;

  ExpectEmitError(ErrorMsg::UNEXPECTED_END_MAP);
}

TEST_F(EmitterErrorTest, InvalidAnchor) {
  out << EMITTER_MANIP::BeginSeq;
  out << Anchor("new\nline") << "Test";
  out << EMITTER_MANIP::EndSeq;

  ExpectEmitError(ErrorMsg::INVALID_ANCHOR);
}

TEST_F(EmitterErrorTest, InvalidAlias) {
  out << EMITTER_MANIP::BeginSeq;
  out << Alias("new\nline");
  out << EMITTER_MANIP::EndSeq;

  ExpectEmitError(ErrorMsg::INVALID_ALIAS);
}
}  // namespace
}  // namespace YAML

#include "yaml-cpp/parser.h"
#include "yaml-cpp/exceptions.h"
#include "mock_event_handler.h"
#include "gtest/gtest.h"

using YAML::Parser;
using YAML::MockEventHandler;
using ::testing::NiceMock;
using ::testing::StrictMock;

TEST(ParserTest, Empty) {
    Parser parser;

    EXPECT_FALSE(parser);

    StrictMock<MockEventHandler> handler;
    EXPECT_FALSE(parser.HandleNextDocument(handler));
}

TEST(ParserTest, CVE_2017_11692) {
    std::istringstream input{"!2"};
    Parser parser{input};

    NiceMock<MockEventHandler> handler;
    EXPECT_THROW(parser.HandleNextDocument(handler), YAML::ParserException);
}

#include "yaml-cpp/parser.h"
#include "mock_event_handler.h"
#include "gtest/gtest.h"

using YAML::Parser;
using YAML::MockEventHandler;

TEST(ParserTest, Empty) {
    Parser parser;

    EXPECT_FALSE(parser);

    MockEventHandler handler;
    EXPECT_FALSE(parser.HandleNextDocument(handler));
    
    std::ostringstream oss;
    parser.PrintTokens(oss);

    EXPECT_EQ(oss.str(), "");
}

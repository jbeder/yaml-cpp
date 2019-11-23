#include "yaml-cpp/parser.h"
#include "mock_event_handler.h"
#include "gtest/gtest.h"

using YAML::Parser;
using YAML::MockEventHandler;
using ::testing::StrictMock;

TEST(ParserTest, Empty) {
    Parser parser;

    EXPECT_FALSE(parser);

    StrictMock<MockEventHandler> handler;
    EXPECT_FALSE(parser.HandleNextDocument(handler));
}

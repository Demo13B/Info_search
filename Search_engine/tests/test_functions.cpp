#include <gtest/gtest.h>
#include "../libs/functions/functions.hpp"

TEST(TokenizerTest, BasicWords) {
    std::vector<std::string> tokens;
    tokenize("Hello world", tokens);

    ASSERT_EQ(tokens.size(), 2);
    EXPECT_EQ(tokens[0], "hello");
    EXPECT_EQ(tokens[1], "world");
}

TEST(TokenizerTest, PunctuationIgnored) {
    std::vector<std::string> tokens;
    tokenize("Hello, world!!!", tokens);

    EXPECT_EQ(tokens, (std::vector<std::string>{"hello", "world"}));
}

TEST(TokenizerTest, HyphenInsideWord) {
    std::vector<std::string> tokens;
    tokenize("state-of-the-art", tokens);

    EXPECT_EQ(tokens.size(), 1);
    EXPECT_EQ(tokens[0], "state-of-the-art");
}

TEST(TokenizerTest, ApostropheInsideWord) {
    std::vector<std::string> tokens;
    tokenize("don't stop", tokens);

    EXPECT_EQ(tokens, (std::vector<std::string>{"don't", "stop"}));
}

TEST(TokenizerTest, NumbersMixed) {
    std::vector<std::string> tokens;
    tokenize("Version 2.0 released", tokens);

    EXPECT_EQ(tokens, (std::vector<std::string>{"version", "2.0", "released"}));
}

TEST(TokenizerTest, EmptyString) {
    std::vector<std::string> tokens;
    tokenize("", tokens);
    EXPECT_TRUE(tokens.empty());
}

TEST(IntersectTest, BasicIntersection) {
    std::vector<std::string> a = {"apple", "banana", "cherry"};
    std::vector<std::string> b = {"banana", "cherry", "date"};

    auto result = intersect(a, b);

    std::vector<std::string> expected = {"banana", "cherry"};
    EXPECT_EQ(result, expected);
}

TEST(IntersectTest, NoIntersection) {
    std::vector<std::string> a = {"apple", "banana"};
    std::vector<std::string> b = {"cherry", "date"};

    auto result = intersect(a, b);

    EXPECT_TRUE(result.empty());
}

TEST(IntersectTest, EmptyVectors) {
    std::vector<std::string> a;
    std::vector<std::string> b;

    auto result = intersect(a, b);

    EXPECT_TRUE(result.empty());
}

TEST(IntersectTest, OneEmptyVector) {
    std::vector<std::string> a = {"apple", "banana"};
    std::vector<std::string> b;

    auto result = intersect(a, b);

    EXPECT_TRUE(result.empty());
}

TEST(IntersectTest, WithDuplicates) {
    std::vector<std::string> a = {"apple", "banana", "banana", "cherry"};
    std::vector<std::string> b = {"banana", "banana", "cherry", "cherry"};

    auto result = intersect(a, b);

    std::vector<std::string> expected = {"banana", "banana", "cherry"};
    EXPECT_EQ(result, expected);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
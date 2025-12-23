#include <gtest/gtest.h>
#include "../libs/parser/parser.hpp"

TEST(ParserTest, ExtractSimpleParagraphs) {
    const char html[] =
        "<html><body>"
        "<div class='article'>"
        "<p>Hello world</p>"
        "<p>Second paragraph</p>"
        "</div>"
        "</body></html>";

    std::vector<char> source(html, html + sizeof(html) - 1);
    Parser p;

    std::string text = p.extract_text(source);

    EXPECT_NE(text.find("Hello world"), std::string::npos);
    EXPECT_NE(text.find("Second paragraph"), std::string::npos);
}

TEST(ParserTest, TrimWorks) {
    Parser p;
    std::string s = "   hello world  ";
    p.trim(s);
    EXPECT_EQ(s, "hello world");
}

TEST(ParserTest, NormalizeWhitespace) {
    Parser p;
    std::string s = "hello   world\t\tagain";
    p.normalize_whitespace(s);
    EXPECT_EQ(s, "hello world again");
}

TEST(ParserTest, UnicodeApostrophe) {
    Parser p;
    std::string s = "don\xE2\x80\x99t do that";
    p.normalize_whitespace(s);
    EXPECT_EQ(s, "don't do that");
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
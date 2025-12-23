#pragma once

#include <gumbo.h>

#include <cctype>
#include <iostream>
#include <string>
#include <vector>

class Parser {
   private:
    GumboNode* find_article_div(GumboNode* node);
    void extract_p_text(GumboNode* node, std::string& out);
    void trim(std::string& s);
    void normalize_whitespace(std::string& s);

   public:
    std::string extract_text(std::vector<char>& source);

    friend class ParserTest_TrimWorks_Test;
    friend class ParserTest_NormalizeWhitespace_Test;
    friend class ParserTest_UnicodeApostrophe_Test;
};
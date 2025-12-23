#include "parser.hpp"
GumboNode* Parser::find_article_div(GumboNode* node) {
    if (!node || node->type != GUMBO_NODE_ELEMENT)
        return nullptr;

    if (node->v.element.tag == GUMBO_TAG_DIV) {
        if (auto* cls =
                gumbo_get_attribute(&node->v.element.attributes, "class")) {
            if (std::string(cls->value).find("article") != std::string::npos)
                return node;
        }

        if (auto* id =
                gumbo_get_attribute(&node->v.element.attributes, "id")) {
            if (std::string(id->value) == "therace-post-content")
                return node;
        }
    }

    GumboVector* children = &node->v.element.children;
    for (size_t i = 0; i < children->length; ++i) {
        if (auto* found =
                find_article_div(static_cast<GumboNode*>(children->data[i])))
            return found;
    }

    return nullptr;
}

void Parser::extract_p_text(GumboNode* node, std::string& out) {
    if (!node || node->type != GUMBO_NODE_ELEMENT)
        return;

    GumboVector* children = &node->v.element.children;

    for (size_t i = 0; i < children->length; ++i) {
        GumboNode* c = static_cast<GumboNode*>(children->data[i]);

        if (c->type == GUMBO_NODE_ELEMENT &&
            c->v.element.tag == GUMBO_TAG_P) {
            GumboVector* pchildren = &c->v.element.children;
            for (size_t j = 0; j < pchildren->length; ++j) {
                GumboNode* t =
                    static_cast<GumboNode*>(pchildren->data[j]);

                if (t->type == GUMBO_NODE_TEXT) {
                    out += t->v.text.text;
                }
            }
            out += "\n\n";
        }

        else if (c->type == GUMBO_NODE_ELEMENT) {
            extract_p_text(c, out);
        }
    }
}

void Parser::trim(std::string& s) {
    s.erase(s.begin(),
            std::find_if(s.begin(), s.end(),
                         [](unsigned char ch) {
                             return !std::isspace(ch);
                         }));

    s.erase(std::find_if(s.rbegin(), s.rend(),
                         [](unsigned char ch) {
                             return !std::isspace(ch);
                         })
                .base(),
            s.end());
}

void Parser::normalize_whitespace(std::string& s) {
    bool in_space = false;
    for (size_t i = 0; i < s.size(); ++i) {
        unsigned char c = static_cast<unsigned char>(s[i]);

        if (std::isspace(c)) {
            if (!in_space) {
                s[i] = ' ';
                in_space = true;
            } else {
                s.erase(i--, 1);
            }
        } else if (c == 0xE2 && i + 2 < s.size() &&
                   static_cast<unsigned char>(s[i + 1]) == 0x80 &&
                   static_cast<unsigned char>(s[i + 2]) == 0x99) {
            s[i] = '\'';
            s.erase(i + 1, 2);
        } else {
            in_space = false;
        }
    }

    size_t pos;
    while ((pos = s.find("\n \n")) != std::string::npos)
        s.replace(pos, 3, "\n\n");
}

std::string Parser::extract_text(std::vector<char>& source) {
    GumboOutput* output = gumbo_parse_with_options(
        &kGumboDefaultOptions,
        source.data(),
        source.size());

    GumboNode* article = find_article_div(output->root);
    if (!article) {
        std::cerr << "Article div not found\n";
    }

    std::string text;
    extract_p_text(article, text);

    trim(text);
    normalize_whitespace(text);

    gumbo_destroy_output(&kGumboDefaultOptions, output);
    return text;
}
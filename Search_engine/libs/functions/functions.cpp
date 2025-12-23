#include "functions.hpp"

std::vector<char> gunzip(const uint8_t* data, size_t size) {
    z_stream zs{};
    zs.next_in = const_cast<Bytef*>(data);
    zs.avail_in = size;

    if (inflateInit2(&zs, 16 + MAX_WBITS) != Z_OK)
        throw std::runtime_error("inflateInit failed");

    std::vector<char> out;
    char buffer[32768];

    int ret;
    do {
        zs.next_out = reinterpret_cast<Bytef*>(buffer);
        zs.avail_out = sizeof(buffer);

        ret = inflate(&zs, 0);
        out.insert(out.end(), buffer, buffer + (sizeof(buffer) - zs.avail_out));
    } while (ret == Z_OK);

    inflateEnd(&zs);

    if (ret != Z_STREAM_END)
        throw std::runtime_error("inflate failed");

    return out;
}

void tokenize(const std::string& text, std::vector<std::string>& tokens) {
    std::string current;
    current.reserve(32);
    const size_t n = text.size();

    for (size_t i = 0; i < n; ++i) {
        char c = std::tolower(static_cast<unsigned char>(text[i]));

        if (std::isalnum(static_cast<unsigned char>(c))) {
            current.push_back(c);
        } else if (c == '-' || c == '.' || c == ':' || c == '\'') {
            if (!current.empty() &&
                i + 1 < n &&
                std::isalnum(static_cast<unsigned char>(current.back())) &&
                std::isalnum(static_cast<unsigned char>(text[i + 1]))) {
                current.push_back(c);
            } else {
                if (!current.empty()) {
                    tokens.push_back(current);
                    current.clear();
                }
            }
        } else if (c == '\'') {
            if (!current.empty() &&
                i + 1 < n &&
                std::isalpha(static_cast<unsigned char>(current.back())) &&
                std::isalpha(static_cast<unsigned char>(text[i + 1]))) {
                current.push_back(c);
            } else {
                if (!current.empty()) {
                    tokens.push_back(current);
                    current.clear();
                }
            }
        } else {
            if (!current.empty()) {
                tokens.push_back(current);
                current.clear();
            }
        }
    }

    if (!current.empty()) {
        tokens.push_back(current);
    }
}

std::vector<std::string> intersect(std::vector<std::string>& a, std::vector<std::string>& b) {
    std::sort(a.begin(), a.end());
    std::sort(b.begin(), b.end());

    std::vector<std::string> result;

    size_t i = 0, j = 0;
    while (i < a.size() && j < b.size()) {
        if (a[i] == b[j]) {
            result.push_back(a[i]);
            ++i;
            ++j;
        } else if (a[i] < b[j]) {
            ++i;
        } else {
            ++j;
        }
    }
    return result;
}

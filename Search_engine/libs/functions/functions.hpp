#pragma once

#include <string>
#include <vector>

#include <zlib.h>

std::vector<char> gunzip(const uint8_t* data, size_t size);
void tokenize(const std::string& text, std::vector<std::string>& tokens);
std::vector<std::string> intersect(std::vector<std::string>& a, std::vector<std::string>& b);
#pragma once

#include <wn.h>
#include <zlib.h>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

struct Node {
    std::string lemma;
    std::vector<std::string> doc_ids;
};

class Indexer {
   private:
    std::vector<Node> index;
    bool sorted = false;
    std::string lemmatize(const std::string& word);
    long find_token(const std::string& token);
    long find_token_bin(const std::string& token);
    void add_doc(const std::string& token, const std::string& doc_id);
    void sort_index();

   public:
    Indexer();
    const std::vector<std::string>& operator[](const std::string& token);
    void build_index(const std::string& doc_id, const std::vector<std::string>& tokens);
    void save_index(const std::string& filename);
    void load_index(const std::string& filename);
    size_t size();

    friend class IndexerTest_AddSingleToken_Test;
    friend class IndexerTest_AddMultipleDocs_Test;
    friend class IndexerTest_FindTokenBinWorks_Test;
    friend class IndexerTest_Lemmatize_Test;
};

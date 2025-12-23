#pragma once

#include <wn.h>
#include <zlib.h>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

struct TF {
    std::string doc_id;
    size_t count;
};

struct Node {
    std::string lemma;
    std::vector<TF> doc_ids;
};

class TF_IDF_Indexer {
   private:
    std::vector<Node> index;
    bool sorted = false;
    std::string lemmatize(const std::string& word);
    long find_token(const std::string& token);
    long find_token_bin(const std::string& token);
    void add_doc(const std::string& token, const std::string& doc_id);
    void sort_index();
    void compress_file(const std::string& input, const ::std::string& output);
    void decompress_file(const std::string& input, const ::std::string& output);

   public:
    TF_IDF_Indexer();
    const std::vector<TF>& operator[](const std::string& token);
    void build_index(const std::string& doc_id, const std::vector<std::string>& tokens);
    void save_index(const std::string& filename);
    void load_index(const std::string& filename);
    void save_index_compressed(const std::string& filename);
    void load_index_compressed(const std::string& filename);
    void sort_tf();
    size_t size();

    friend class Scorer;

    friend class TF_IDF_IndexerTest_AddSingleToken_Test;
    friend class TF_IDF_IndexerTest_AddMultipleDocs_Test;
    friend class TF_IDF_IndexerTest_FindTokenBinWorks_Test;
    friend class TF_IDF_IndexerTest_SortTFWorks_Test;
};

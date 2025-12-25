#pragma once

#include <wn.h>
#include <zlib.h>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include <iostream>

struct TF {
    std::string doc_id;
    size_t count;
};

struct Node {
    std::string lemma;
    std::vector<TF> doc_ids;
};

struct Node_Compressed {
    std::string lemma;
    std::vector<uint8_t> postings;
};

class TF_IDF_Indexer {
   private:
    std::vector<Node> index;
    std::vector<Node_Compressed> index_compressed;
    std::vector<std::string> ids;
    bool sorted = false;
    bool compressed = false;
    std::string lemmatize(const std::string& word);
    long find_token(const std::string& token);
    long find_token_bin(const std::string& token);
    void add_doc(const std::string& token, const std::string& doc_id);
    void sort_index();
    void compress_file(const std::string& input, const ::std::string& output);
    void decompress_file(const std::string& input, const ::std::string& output);
    long find_mongo_id(const std::string& mongo_id);
    void decompress_posting(const Node_Compressed& node, std::vector<TF>& res);

   public:
    TF_IDF_Indexer();
    std::vector<TF> operator[](const std::string& token);
    void build_index(const std::string& doc_id, const std::vector<std::string>& tokens);
    void save_index(const std::string& filename);
    void load_index(const std::string& filename, bool is_compressed);
    void save_index_compressed(const std::string& filename);
    void load_index_compressed(const std::string& filename);
    void sort_tf();
    size_t size();

    friend class Scorer;

    friend class TF_IDF_IndexerTest_AddSingleToken_Test;
    friend class TF_IDF_IndexerTest_AddMultipleDocs_Test;
    friend class TF_IDF_IndexerTest_FindTokenBinWorks_Test;
    friend class TF_IDF_IndexerTest_SortTFWorks_Test;
    friend class TF_IDF_IndexerTest_Compression_Test;

    void encode_vb(size_t x, std::vector<uint8_t>& out);
    size_t decode_vb(const std::vector<uint8_t>& in, size_t& pos);
    void compress();
};

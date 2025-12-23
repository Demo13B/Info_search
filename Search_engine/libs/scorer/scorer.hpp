#pragma once

#include <string>
#include <vector>

#include "../tf-idf_indexer/tf-idf_indexer.hpp"

struct ScoreEntry {
    std::string doc_id;
    double score;
};

class Scorer {
   private:
    std::vector<ScoreEntry> scoreboard;
    TF_IDF_Indexer& index;
    size_t docs_total;

    void add_score(const std::string& doc_id, double score);

   public:
    Scorer(size_t N, TF_IDF_Indexer& id);
    void score_token(const std::string& token);
    std::vector<ScoreEntry> show_scoreboard();
    void clear();
};
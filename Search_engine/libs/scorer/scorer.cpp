#include "scorer.hpp"

void Scorer::add_score(const std::string& doc_id, double score) {
    for (size_t i = 0; i != scoreboard.size(); ++i) {
        if (scoreboard[i].doc_id == doc_id) {
            scoreboard[i].score += score;
            return;
        }
    }

    ScoreEntry se;
    se.doc_id = doc_id;
    se.score = score;
    scoreboard.push_back(se);
}

Scorer::Scorer(size_t N, TF_IDF_Indexer& id) : index(id) {
    docs_total = N;
}

void Scorer::score_token(const std::string& token) {
    std::string lemma = index.lemmatize(token);
    auto data = index[lemma];
    size_t df = data.size();

    if (df > 0.5 * docs_total)
        return;

    double idf = std::log((double)(docs_total + 1) / (double)(df + 1)) + 1;

    size_t limit = std::min(100ul, df);
    for (size_t i = 0; i <= limit; ++i) {
        double tf = std::log((double)data[i].count) + 1;
        double score = tf * idf;
        add_score(data[i].doc_id, score);
    }
}

std::vector<ScoreEntry> Scorer::show_scoreboard() {
    std::sort(scoreboard.begin(), scoreboard.end(), [](const ScoreEntry& a, const ScoreEntry& b) {
        return a.score > b.score;
    });

    if (scoreboard.size() > 100) {
        scoreboard.resize(100);
    }

    return scoreboard;
}

void Scorer::clear() {
    scoreboard.resize(0);
}

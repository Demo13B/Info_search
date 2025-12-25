#include <gtest/gtest.h>
#include "../libs/scorer/scorer.hpp"

TEST(ScorerTest, AddScoreAndShowScoreboard) {
    TF_IDF_Indexer ind;

    ind.build_index("doc1", {"apple", "banana", "cherry"});
    ind.build_index("doc2", {"apple", "banana"});
    ind.build_index("doc3", {"apple", "date"});

    Scorer scorer(3, ind);

    scorer.score_token("apple");
    scorer.score_token("banana");
    scorer.score_token("cherry");

    auto board = scorer.show_scoreboard();

    ASSERT_FALSE(board.empty());

    std::vector<std::string> doc_ids;
    for (auto& e : board) {
        doc_ids.push_back(e.doc_id);
    }

    EXPECT_NE(std::find(doc_ids.begin(), doc_ids.end(), "doc1"), doc_ids.end());
    EXPECT_EQ(std::find(doc_ids.begin(), doc_ids.end(), "doc2"), doc_ids.end());
    EXPECT_EQ(std::find(doc_ids.begin(), doc_ids.end(), "doc3"), doc_ids.end());
}

TEST(ScorerTest, ScoreboardSorting) {
    TF_IDF_Indexer indexer;

    indexer.build_index("doc1", {"apple", "banana"});
    indexer.build_index("doc2", {"apple"});
    indexer.build_index("doc3", {"apple", "banana", "cherry"});

    Scorer scorer(3, indexer);

    scorer.score_token("apple");
    scorer.score_token("banana");
    scorer.score_token("cherry");

    auto board = scorer.show_scoreboard();

    for (size_t i = 1; i < board.size(); ++i) {
        EXPECT_GE(board[i - 1].score, board[i].score);
    }
}

TEST(ScorerTest, ClearScoreboardWorks) {
    TF_IDF_Indexer indexer;

    indexer.build_index("doc1", {"apple"});
    indexer.build_index("doc2", {"banana"});

    Scorer scorer(2, indexer);

    scorer.score_token("apple");
    scorer.score_token("banana");

    auto board_before = scorer.show_scoreboard();
    EXPECT_FALSE(board_before.empty());

    scorer.clear();

    auto board_after = scorer.show_scoreboard();
    EXPECT_TRUE(board_after.empty());
}

TEST(ScorerTest, HighDocumentFrequencyTokenIgnored) {
    TF_IDF_Indexer indexer;

    indexer.build_index("doc1", {"common"});
    indexer.build_index("doc2", {"common"});
    indexer.build_index("doc3", {"common"});

    Scorer scorer(3, indexer);

    scorer.score_token("common");

    auto board = scorer.show_scoreboard();

    EXPECT_TRUE(board.empty());
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
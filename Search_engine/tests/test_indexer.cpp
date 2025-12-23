#include <gtest/gtest.h>
#include "../libs/indexer/indexer.hpp"

TEST(IndexerTest, AddSingleToken) {
    Indexer idx;
    idx.add_doc("running", "doc1");

    auto docs = idx["running"];
    ASSERT_EQ(docs.size(), 1);
    EXPECT_EQ(docs[0], "doc1");
}

TEST(IndexerTest, AddMultipleDocs) {
    Indexer idx;
    idx.add_doc("running", "doc1");
    idx.add_doc("running", "doc2");
    idx.add_doc("running", "doc2");

    auto docs = idx["running"];
    ASSERT_EQ(docs.size(), 2);
    EXPECT_EQ(docs[0], "doc1");
    EXPECT_EQ(docs[1], "doc2");
}

TEST(IndexerTest, FindTokenBinWorks) {
    Indexer idx;
    idx.add_doc("apple", "doc1");
    idx.add_doc("banana", "doc2");
    idx.sort_index();

    EXPECT_NE(idx.find_token_bin("apple"), -1);
    EXPECT_NE(idx.find_token_bin("banana"), -1);
    EXPECT_EQ(idx.find_token_bin("cherry"), -1);
}

TEST(IndexerTest, BuildIndexMultipleTokens) {
    Indexer idx;
    std::vector<std::string> tokens = {"run", "jump", "walk"};
    idx.build_index("doc1", tokens);

    for (const auto& t : tokens) {
        auto docs = idx[t];
        ASSERT_EQ(docs.size(), 1);
        EXPECT_EQ(docs[0], "doc1");
    }
}

TEST(IndexerTest, SaveLoadIndex) {
    Indexer idx;
    idx.build_index("doc1", {"run", "jump"});
    idx.build_index("doc2", {"run", "walk"});

    std::string filename = "test_index.bin";
    idx.save_index(filename);

    Indexer idx2;
    idx2.load_index(filename);

    EXPECT_EQ(idx.size(), idx2.size());
    EXPECT_EQ(idx["run"], idx2["run"]);
    EXPECT_EQ(idx["jump"], idx2["jump"]);
    EXPECT_EQ(idx["walk"], idx2["walk"]);

    std::filesystem::remove(filename);
}

TEST(IndexerTest, IndexSize) {
    Indexer idx;
    idx.build_index("doc1", {"a", "b", "c"});
    EXPECT_EQ(idx.size(), 3);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
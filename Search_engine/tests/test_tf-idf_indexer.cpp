#include <gtest/gtest.h>
#include "../libs/tf-idf_indexer/tf-idf_indexer.hpp"

TEST(TF_IDF_IndexerTest, AddSingleToken) {
    TF_IDF_Indexer idx;
    idx.add_doc("running", "doc1");

    auto docs = idx["running"];
    ASSERT_EQ(docs.size(), 1);
    EXPECT_EQ(docs[0].doc_id, "doc1");
    EXPECT_EQ(docs[0].count, 1);
}

TEST(TF_IDF_IndexerTest, AddMultipleDocs) {
    TF_IDF_Indexer idx;
    idx.add_doc("running", "doc1");
    idx.add_doc("running", "doc2");
    idx.add_doc("running", "doc2");

    auto docs = idx["run"];
    ASSERT_EQ(docs.size(), 2);

    EXPECT_EQ(docs[0].doc_id, "doc1");
    EXPECT_EQ(docs[0].count, 1);

    EXPECT_EQ(docs[1].doc_id, "doc2");
    EXPECT_EQ(docs[1].count, 2);
}

TEST(TF_IDF_IndexerTest, FindTokenBinWorks) {
    TF_IDF_Indexer idx;
    idx.add_doc("apple", "doc1");
    idx.add_doc("banana", "doc2");
    idx.sort_index();

    EXPECT_NE(idx.find_token_bin("apple"), -1);
    EXPECT_NE(idx.find_token_bin("banana"), -1);
    EXPECT_EQ(idx.find_token_bin("cherry"), -1);
}

TEST(TF_IDF_IndexerTest, BuildIndexMultipleTokens) {
    TF_IDF_Indexer idx;
    std::vector<std::string> tokens = {"run", "jump", "walk"};
    idx.build_index("doc1", tokens);

    for (const auto& t : tokens) {
        auto docs = idx[t];
        ASSERT_EQ(docs.size(), 1);
        EXPECT_EQ(docs[0].doc_id, "doc1");
        EXPECT_EQ(docs[0].count, 1);
    }
}

TEST(TF_IDF_IndexerTest, SaveLoadIndex) {
    TF_IDF_Indexer idx;
    idx.build_index("doc1", {"run", "jump"});
    idx.build_index("doc2", {"run", "walk"});

    std::string filename = "test_index.bin";
    idx.save_index(filename);

    TF_IDF_Indexer idx2;
    idx2.load_index(filename);

    EXPECT_EQ(idx.size(), idx2.size());

    for (const auto& t : {"run", "jump", "walk"}) {
        auto docs1 = idx[t];
        auto docs2 = idx2[t];
        ASSERT_EQ(docs1.size(), docs2.size());

        for (size_t i = 0; i < docs1.size(); ++i) {
            EXPECT_EQ(docs1[i].doc_id, docs2[i].doc_id);
            EXPECT_EQ(docs1[i].count, docs2[i].count);
        }
    }

    std::filesystem::remove(filename);
}

TEST(TF_IDF_IndexerTest, SaveLoadIndexCompressed) {
    TF_IDF_Indexer idx;
    idx.build_index("doc1", {"run", "jump"});
    idx.build_index("doc2", {"run", "walk"});

    std::string filename = "test_index.gz";
    idx.save_index_compressed(filename);

    TF_IDF_Indexer idx2;
    idx2.load_index_compressed(filename);

    EXPECT_EQ(idx.size(), idx2.size());

    for (const auto& t : {"run", "jump", "walk"}) {
        auto docs1 = idx[t];
        auto docs2 = idx2[t];
        ASSERT_EQ(docs1.size(), docs2.size());

        for (size_t i = 0; i < docs1.size(); ++i) {
            EXPECT_EQ(docs1[i].doc_id, docs2[i].doc_id);
            EXPECT_EQ(docs1[i].count, docs2[i].count);
        }
    }

    std::filesystem::remove(filename);
}

TEST(TF_IDF_IndexerTest, IndexSize) {
    TF_IDF_Indexer idx;
    idx.build_index("doc1", {"a", "b", "c"});
    EXPECT_EQ(idx.size(), 3);
}

TEST(TF_IDF_IndexerTest, SortTFWorks) {
    TF_IDF_Indexer idx;

    idx.add_doc("apple", "doc1");
    idx.add_doc("apple", "doc1");
    idx.add_doc("apple", "doc2");
    idx.add_doc("apple", "doc2");
    idx.add_doc("apple", "doc2");
    idx.add_doc("apple", "doc3");

    auto docs_before = idx["apple"];
    ASSERT_EQ(docs_before.size(), 3);
    EXPECT_EQ(docs_before[0].doc_id, "doc1");
    EXPECT_EQ(docs_before[0].count, 2);
    EXPECT_EQ(docs_before[1].doc_id, "doc2");
    EXPECT_EQ(docs_before[1].count, 3);
    EXPECT_EQ(docs_before[2].doc_id, "doc3");
    EXPECT_EQ(docs_before[2].count, 1);

    idx.sort_tf();

    auto docs_after = idx["apple"];
    ASSERT_EQ(docs_after.size(), 3);

    EXPECT_EQ(docs_after[0].doc_id, "doc2");
    EXPECT_EQ(docs_after[0].count, 3);

    EXPECT_EQ(docs_after[1].doc_id, "doc1");
    EXPECT_EQ(docs_after[1].count, 2);

    EXPECT_EQ(docs_after[2].doc_id, "doc3");
    EXPECT_EQ(docs_after[2].count, 1);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
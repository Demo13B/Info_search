#include <iomanip>
#include <iostream>

#include <libs/db/db.hpp>
#include <libs/tf-idf_indexer/tf-idf_indexer.hpp>

int main() {
    auto db = DB("mongodb://admin:bamboo@demo13b.ddnsfree.com:21000");
    auto id = TF_IDF_Indexer();

    auto cursor = db.getCursor();

    std::cout << "Counting documents....";
    size_t total_documents = db.countDocuments();
    std::cout << "Completed\nFound: " << total_documents << " documents\n";
    std::cout << "Beginning Tf IDF index build....\n";
    size_t counter = 1;

    try {
        for (const auto& doc : cursor) {
            std::cout << "------------------------------------------------------------------\n";
            std::cout << "Indexing document at: " << doc["url"].get_string().value << "\n";

            std::string doc_id = doc["_id"].get_oid().value.to_string();

            bsoncxx::array::view tokens = doc["tokens"].get_array().value;

            std::vector<std::string> token_array;
            for (const auto& token : tokens) {
                auto string_view = token.get_string().value;
                token_array.push_back(std::string(string_view.data(), string_view.size()));
            }

            id.build_index(doc_id, token_array);

            std::cout << "Completed\n";
            double percentage = (double(counter) / total_documents) * 100.0;
            std::cout << std::fixed << std::setprecision(2) << "Indexed: " << percentage << "% (" << counter << " of " << total_documents << " documents)\n";
            counter++;
        }

        std::cout << "Sorting term frequencies.... ";
        id.sort_tf();
        std::cout << "Sorted\n";

        std::cout << "Saving TF IDF index to file....";
        id.save_index_compressed("/Users/timursalihov/Inf_search/Search_engine/tf-idf_index.bin");
        std::cout << "Saved\n";
    } catch (...) {
        std::cerr << "MongoDB error: ";
        id.save_index("/Users/timursalihov/Inf_search/Search_engine/tf-idf_index.bin");
    }
}
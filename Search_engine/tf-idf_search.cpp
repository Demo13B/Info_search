#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>

#include <libs/db/db.hpp>
#include <libs/functions/functions.hpp>
#include <libs/scorer/scorer.hpp>
#include <libs/tf-idf_indexer/tf-idf_indexer.hpp>

int main() {
    auto id = TF_IDF_Indexer();
    id.load_index_compressed("/Users/timursalihov/Inf_search/Search_engine/tf-idf_index.gz");

    auto db = DB("mongodb://admin:bamboo@demo13b.ddnsfree.com:21000");

    Scorer scorer = Scorer(db.countDocuments(), id);

    std::cout << "Welcome to F1 news search system. To exit type exit()\n";

    while (true) {
        std::string query;
        std::cout << "Search query: ";
        std::getline(std::cin, query);

        if (query == "exit()") {
            std::cout << "Thank you. Goodbye.\n";
            break;
        } else {
            std::vector<std::string> tokenised;
            tokenize(query, tokenised);

            if (tokenised.size() == 0) {
                continue;
            }

            std::cout << "Searching....\n";

            for (const auto& token : tokenised) {
                scorer.score_token(token);
            }

            auto result = scorer.show_scoreboard();
            scorer.clear();

            if (result.size() == 0) {
                std::cout << "No articles found\n";
            } else {
                int pages = (result.size() + 9) / 10;
                std::cout << "Found " << pages << " pages\n";
                std::cout << "Type page number to see results. To exit type -1\n";

                while (true) {
                    int page_num;
                    std::cin >> page_num;
                    std::cin.ignore();

                    if (page_num == -1) {
                        break;
                    }

                    std::cout << "Showing page " << page_num << " out of " << pages << ":\n";
                    for (size_t i = 0; i != 10; ++i) {
                        if ((page_num - 1) * 10 + i < result.size()) {
                            std::cout << db.getURLbyID(result[(page_num - 1) * 10 + i].doc_id) << "\n";
                        }
                    }
                }
            }
        }
    }
}
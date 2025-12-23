#include <algorithm>
#include <iostream>
#include <vector>

#include <libs/db/db.hpp>
#include <libs/functions/functions.hpp>
#include <libs/indexer/indexer.hpp>

int main() {
    auto id = Indexer();
    id.load_index("/Users/timursalihov/Inf_search/Search_engine/index.bin");

    auto db = DB("mongodb://admin:bamboo@demo13b.ddnsfree.com:21000");

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

            std::vector<std::string> result = id[tokenised[0]];

            for (auto& token : tokenised) {
                std::vector<std::string> word_res = id[token];
                result = intersect(word_res, result);
            }

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
                            std::cout << db.getURLbyID(result[(page_num - 1) * 10 + i]) << "\n";
                        }
                    }
                }
            }
        }
    }
}
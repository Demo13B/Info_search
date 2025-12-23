#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

#include <libs/db/db.hpp>
#include <libs/functions/functions.hpp>
#include <libs/parser/parser.hpp>

int main() {
    auto db = DB("mongodb://admin:bamboo@demo13b.ddnsfree.com:21000");
    Parser ps = Parser();

    std::cout << "Counting documents....";
    size_t total_documents = db.countDocuments();
    std::cout << "Completed\nFound: " << total_documents << " documents\n";
    std::cout << "Preparing for tokenization....\n";
    size_t counter = 1;

    auto cursor = db.getCursor();
    for (const auto& doc : cursor) {
        std::cout << "------------------------------------------------------------------\n";
        std::cout << "Tokenizing document at: " << doc["url"].get_string().value << "\n";

        auto binary = doc["data"].get_binary();
        auto decompressed = gunzip(binary.bytes, binary.size);

        std::string text = ps.extract_text(decompressed);

        std::vector<std::string> tokenized;
        tokenize(text, tokenized);

        db.addTokens(doc, tokenized);

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        std::cout << "Completed\n";
        std::cout << "Tokenized: " << counter << " of " << total_documents << " documents\n";
        counter++;
    }
}

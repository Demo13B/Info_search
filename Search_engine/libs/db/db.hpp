#pragma once

#include <bsoncxx/builder/basic/array.hpp>
#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/document/view.hpp>
#include <bsoncxx/json.hpp>
#include <bsoncxx/types.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/uri.hpp>

class DB {
   private:
    static mongocxx::instance instance;
    mongocxx::client client;
    mongocxx::collection collection;

   public:
    explicit DB(const std::string url);
    bsoncxx::v_noabi::stdx::optional<bsoncxx::v_noabi::document::value> findByURL(const std::string url);
    std::string getURLbyID(const std::string id);
    mongocxx::cursor getCursor();
    void addTokens(const bsoncxx::v_noabi::document::view& doc, const std::vector<std::string>& tokenized);
    size_t countDocuments();
};
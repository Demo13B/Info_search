#include "db.hpp"

mongocxx::instance DB::instance{};

DB::DB(const std::string url)
    : client{mongocxx::uri{url}},
      collection{client["f1crawler"]["documents"]} {
}

bsoncxx::v_noabi::stdx::optional<bsoncxx::v_noabi::document::value> DB::findByURL(const std::string url) {
    auto result = collection.find_one(
        bsoncxx::builder::basic::make_document(
            bsoncxx::builder::basic::kvp(
                "url",
                url)));

    if (!result) {
        std::cerr << "Document not found\n";
    }

    return result;
}

mongocxx::cursor DB::getCursor() {
    mongocxx::options::find opts;
    opts.no_cursor_timeout(true);
    opts.batch_size(1000);
    return this->collection.find({}, opts);
}

void DB::addTokens(const bsoncxx::v_noabi::document::view& doc, const std::vector<std::string>& tokenized) {
    bsoncxx::builder::basic::array token_array;
    for (const auto& t : tokenized) {
        token_array.append(t);
    }

    auto filter = bsoncxx::builder::basic::make_document(
        bsoncxx::builder::basic::kvp("_id", doc["_id"].get_oid().value));

    auto update_doc = bsoncxx::builder::basic::make_document(
        bsoncxx::builder::basic::kvp("$set",
                                     bsoncxx::builder::basic::make_document(
                                         bsoncxx::builder::basic::kvp("tokens", token_array))));

    this->collection.update_one(filter.view(), update_doc.view());
}

size_t DB::countDocuments() {
    return this->collection.count_documents({});
}

std::string DB::getURLbyID(const std::string id) {
    auto result = collection.find_one(
        bsoncxx::builder::basic::make_document(
            bsoncxx::builder::basic::kvp(
                "_id",
                bsoncxx::types::b_oid{bsoncxx::oid{id}})));

    if (!result) {
        std::cerr << "Document not found\n";
        return "";
    }

    auto url_view = (*result)["url"].get_string().value;
    std::string url(url_view.data(), url_view.size());

    return url;
}
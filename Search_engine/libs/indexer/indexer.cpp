#include "indexer.hpp"

Indexer::Indexer() {
    wninit();
}

std::string Indexer::lemmatize(const std::string& word) {
    char* lemma;
    for (int i = 1; i != 5; ++i) {
        lemma = morphword((char*)(word.c_str()), i);

        if (lemma)
            break;
    }

    if (lemma) {
        return lemma;
    } else {
        return word;
    }
}

long Indexer::find_token(const std::string& token) {
    std::string lemma = lemmatize(token);

    for (size_t i = 0; i < index.size(); ++i) {
        if (index[i].lemma == lemma)
            return i;
    }
    return -1;
}

long Indexer::find_token_bin(const std::string& token) {
    if (!sorted) {
        sort_index();
    }

    std::string lemma = lemmatize(token);

    long left = 0;
    long right = static_cast<long>(index.size()) - 1;

    while (left <= right) {
        long mid = left + (right - left) / 2;
        const std::string& mid_lemma = index[mid].lemma;

        if (mid_lemma == lemma) {
            return mid;
        } else if (mid_lemma < lemma) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }

    return -1;
}

void Indexer::add_doc(const std::string& token, const std::string& doc_id) {
    std::string lemma = lemmatize(token);
    long pos = find_token(token);

    if (pos == -1) {
        Node n;
        n.lemma = lemma;
        n.doc_ids = {doc_id};
        index.push_back(n);
    } else {
        std::vector<std::string>& docs = index[pos].doc_ids;
        if (docs.empty() || docs.back() != doc_id)
            docs.push_back(doc_id);
    }

    sorted = false;
}

const std::vector<std::string>& Indexer::operator[](const std::string& token) {
    long pos = find_token_bin(token);

    static const std::vector<std::string> empty;

    if (pos == -1)
        return empty;

    return index[pos].doc_ids;
}

void Indexer::build_index(const std::string& doc_id, const std::vector<std::string>& tokens) {
    for (auto& token : tokens) {
        add_doc(token, doc_id);
    }
}

void Indexer::sort_index() {
    std::sort(index.begin(), index.end(), [](const Node& a, const Node& b) {
        return a.lemma < b.lemma;
    });

    sorted = true;
}

void Indexer::save_index(const std::string& filename) {
    std::ofstream out(filename, std::ios::binary);
    size_t n = index.size();
    out.write(reinterpret_cast<const char*>(&n), sizeof(n));

    for (const auto& node : index) {
        size_t lemma_len = node.lemma.size();
        out.write(reinterpret_cast<const char*>(&lemma_len), sizeof(lemma_len));
        out.write(node.lemma.data(), lemma_len);

        size_t doc_count = node.doc_ids.size();
        out.write(reinterpret_cast<const char*>(&doc_count), sizeof(doc_count));

        for (const auto& id : node.doc_ids) {
            size_t id_len = id.size();
            out.write(reinterpret_cast<const char*>(&id_len), sizeof(id_len));
            out.write(id.data(), id_len);
        }
    }
}

void Indexer::load_index(const std::string& filename) {
    std::ifstream in(filename, std::ios::binary);

    size_t n;
    in.read(reinterpret_cast<char*>(&n), sizeof(n));
    index.resize(n);

    for (size_t i = 0; i < n; ++i) {
        size_t lemma_len;
        in.read(reinterpret_cast<char*>(&lemma_len), sizeof(lemma_len));
        index[i].lemma.resize(lemma_len);
        in.read(index[i].lemma.data(), lemma_len);

        size_t doc_count;
        in.read(reinterpret_cast<char*>(&doc_count), sizeof(doc_count));
        index[i].doc_ids.resize(doc_count);

        for (size_t j = 0; j < doc_count; ++j) {
            size_t id_len;
            in.read(reinterpret_cast<char*>(&id_len), sizeof(id_len));
            index[i].doc_ids[j].resize(id_len);
            in.read(index[i].doc_ids[j].data(), id_len);
        }
    }
}

size_t Indexer::size() {
    return index.size();
}
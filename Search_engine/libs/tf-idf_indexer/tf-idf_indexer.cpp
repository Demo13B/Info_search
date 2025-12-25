#include "tf-idf_indexer.hpp"

TF_IDF_Indexer::TF_IDF_Indexer() {
    wninit();
}

std::string TF_IDF_Indexer::lemmatize(const std::string& word) {
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

long TF_IDF_Indexer::find_token(const std::string& token) {
    std::string lemma = lemmatize(token);

    for (size_t i = 0; i < index.size(); ++i) {
        if (index[i].lemma == lemma)
            return i;
    }
    return -1;
}

long TF_IDF_Indexer::find_token_bin(const std::string& token) {
    if (!compressed) {
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

    std::string lemma = lemmatize(token);

    long left = 0;
    long right = static_cast<long>(index_compressed.size()) - 1;

    while (left <= right) {
        long mid = left + (right - left) / 2;
        const std::string& mid_lemma = index_compressed[mid].lemma;

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

void TF_IDF_Indexer::add_doc(const std::string& token, const std::string& doc_id) {
    std::string lemma = lemmatize(token);
    long pos = find_token(token);

    if (pos == -1) {
        TF tf;
        tf.doc_id = doc_id;
        tf.count = 1;

        Node n;
        n.lemma = lemma;
        n.doc_ids = {tf};
        index.push_back(n);
    } else {
        std::vector<TF>& docs = index[pos].doc_ids;
        if (docs.empty() || docs.back().doc_id != doc_id) {
            TF tf;
            tf.doc_id = doc_id;
            tf.count = 1;
            docs.push_back(tf);
        } else {
            docs.back().count++;
        }
    }

    sorted = false;
}

std::vector<TF> TF_IDF_Indexer::operator[](const std::string& token) {
    long pos = find_token_bin(token);

    std::vector<TF> res;

    if (pos == -1)
        return res;

    if (!compressed)
        return index[pos].doc_ids;

    decompress_posting(index_compressed[pos], res);
    return res;
}

void TF_IDF_Indexer::build_index(const std::string& doc_id, const std::vector<std::string>& tokens) {
    for (auto& token : tokens) {
        add_doc(token, doc_id);
    }
}

void TF_IDF_Indexer::sort_index() {
    std::sort(index.begin(), index.end(), [](const Node& a, const Node& b) {
        return a.lemma < b.lemma;
    });

    sorted = true;
}

void TF_IDF_Indexer::save_index(const std::string& filename) {
    std::ofstream out(filename, std::ios::binary);

    if (!compressed) {
        size_t n = index.size();
        out.write(reinterpret_cast<const char*>(&n), sizeof(n));

        for (const auto& node : index) {
            size_t lemma_len = node.lemma.size();
            out.write(reinterpret_cast<const char*>(&lemma_len), sizeof(lemma_len));
            out.write(node.lemma.data(), lemma_len);

            size_t doc_count = node.doc_ids.size();
            out.write(reinterpret_cast<const char*>(&doc_count), sizeof(doc_count));

            for (const auto& id : node.doc_ids) {
                size_t id_doc_id_len = id.doc_id.size();
                out.write(reinterpret_cast<const char*>(&id_doc_id_len), sizeof(id_doc_id_len));
                out.write(id.doc_id.data(), id_doc_id_len);

                size_t id_count = id.count;
                out.write(reinterpret_cast<const char*>(&id_count), sizeof(id_count));
            }
        }

        return;
    }

    size_t n = ids.size();
    out.write(reinterpret_cast<const char*>(&n), sizeof(n));
    for (const auto& id : ids) {
        size_t id_len = id.size();
        out.write(reinterpret_cast<const char*>(&id_len), sizeof(id_len));
        out.write(id.data(), id_len);
    }

    n = index_compressed.size();
    out.write(reinterpret_cast<const char*>(&n), sizeof(n));
    for (const auto& node : index_compressed) {
        size_t lemma_len = node.lemma.size();
        out.write(reinterpret_cast<const char*>(&lemma_len), sizeof(lemma_len));
        out.write(node.lemma.data(), lemma_len);

        size_t postings_len = node.postings.size();
        out.write(reinterpret_cast<const char*>(&postings_len), sizeof(postings_len));
        out.write(reinterpret_cast<const char*>(node.postings.data()), postings_len);
    }
}

void TF_IDF_Indexer::load_index(const std::string& filename, bool is_compressed) {
    std::ifstream in(filename, std::ios::binary);

    compressed = is_compressed;

    if (!compressed) {
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
                size_t id_doc_id_len;
                in.read(reinterpret_cast<char*>(&id_doc_id_len), sizeof(id_doc_id_len));
                index[i].doc_ids[j].doc_id.resize(id_doc_id_len);
                in.read(index[i].doc_ids[j].doc_id.data(), id_doc_id_len);

                size_t id_count;
                in.read(reinterpret_cast<char*>(&id_count), sizeof(id_count));
                index[i].doc_ids[j].count = id_count;
            }
        }

        return;
    }

    size_t n;
    in.read(reinterpret_cast<char*>(&n), sizeof(n));
    ids.resize(n);
    for (size_t i = 0; i < n; ++i) {
        size_t id_len;
        in.read(reinterpret_cast<char*>(&id_len), sizeof(id_len));
        ids[i].resize(id_len);
        in.read(ids[i].data(), id_len);
    }

    in.read(reinterpret_cast<char*>(&n), sizeof(n));
    index_compressed.resize(n);
    for (size_t i = 0; i < n; ++i) {
        size_t lemma_len;
        in.read(reinterpret_cast<char*>(&lemma_len), sizeof(lemma_len));
        index_compressed[i].lemma.resize(lemma_len);
        in.read(index_compressed[i].lemma.data(), lemma_len);

        size_t postings_len;
        in.read(reinterpret_cast<char*>(&postings_len), sizeof(postings_len));
        index_compressed[i].postings.resize(postings_len);
        in.read(reinterpret_cast<char*>(index_compressed[i].postings.data()), postings_len);
    }
}

size_t TF_IDF_Indexer::size() {
    if (!compressed) {
        return index.size();
    }
    return index_compressed.size();
}

void TF_IDF_Indexer::compress_file(const std::string& input, const ::std::string& output) {
    size_t BUF_SIZE = 1 << 15;
    char buffer[BUF_SIZE];

    FILE* in = fopen(input.c_str(), "rb");

    gzFile out = gzopen(output.c_str(), "wb");

    size_t bytes;
    while ((bytes = fread(buffer, 1, BUF_SIZE, in)) > 0) {
        if (gzwrite(out, buffer, bytes) == 0) {
            gzclose(out);
            fclose(in);
        }
    }

    gzclose(out);
    fclose(in);
}

void TF_IDF_Indexer::decompress_file(const std::string& input, const ::std::string& output) {
    size_t BUF_SIZE = 1 << 15;
    char buffer[BUF_SIZE];

    gzFile in = gzopen(input.c_str(), "rb");

    FILE* out = fopen(output.c_str(), "wb");

    int bytes;
    while ((bytes = gzread(in, buffer, BUF_SIZE)) > 0) {
        fwrite(buffer, 1, bytes, out);
    }

    gzclose(in);
    fclose(out);
}

void TF_IDF_Indexer::save_index_compressed(const std::string& filename) {
    std::ofstream out("tmp.bin", std::ios::binary);
    size_t n = index.size();
    out.write(reinterpret_cast<const char*>(&n), sizeof(n));

    for (const auto& node : index) {
        size_t lemma_len = node.lemma.size();
        out.write(reinterpret_cast<const char*>(&lemma_len), sizeof(lemma_len));
        out.write(node.lemma.data(), lemma_len);

        size_t doc_count = node.doc_ids.size();
        out.write(reinterpret_cast<const char*>(&doc_count), sizeof(doc_count));

        for (const auto& id : node.doc_ids) {
            size_t id_doc_id_len = id.doc_id.size();
            out.write(reinterpret_cast<const char*>(&id_doc_id_len), sizeof(id_doc_id_len));
            out.write(id.doc_id.data(), id_doc_id_len);

            size_t id_count = id.count;
            out.write(reinterpret_cast<const char*>(&id_count), sizeof(id_count));
        }
    }

    out.close();
    compress_file("tmp.bin", filename);
    std::filesystem::remove("tmp.bin");
}

void TF_IDF_Indexer::load_index_compressed(const std::string& filename) {
    decompress_file(filename, "tmp.bin");

    std::ifstream in("tmp.bin", std::ios::binary);

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
            size_t id_doc_id_len;
            in.read(reinterpret_cast<char*>(&id_doc_id_len), sizeof(id_doc_id_len));
            index[i].doc_ids[j].doc_id.resize(id_doc_id_len);
            in.read(index[i].doc_ids[j].doc_id.data(), id_doc_id_len);

            size_t id_count;
            in.read(reinterpret_cast<char*>(&id_count), sizeof(id_count));
            index[i].doc_ids[j].count = id_count;
        }
    }

    in.close();
    std::filesystem::remove("tmp.bin");
}

void TF_IDF_Indexer::sort_tf() {
    for (auto& elem : index) {
        std::sort(elem.doc_ids.begin(), elem.doc_ids.end(), [](const TF& a, const TF& b) {
            return a.count > b.count;
        });
    }
}

void TF_IDF_Indexer::encode_vb(size_t x, std::vector<uint8_t>& out) {
    while (x >= 128) {
        out.push_back(x & 0x7F);
        x >>= 7;
    }
    out.push_back(x | 0x80);
}

size_t TF_IDF_Indexer::decode_vb(const std::vector<uint8_t>& in, size_t& pos) {
    size_t x = 0, shift = 0;
    while (pos < in.size()) {
        uint8_t b = in[pos++];
        x |= (b & 0x7F) << shift;
        if (b & 0x80) {
            break;
        }
        shift += 7;
    }
    return x;
}

long TF_IDF_Indexer::find_mongo_id(const std::string& mongo_id) {
    for (size_t i = 0; i != ids.size(); ++i) {
        if (ids[i] == mongo_id)
            return i;
    }
    return -1;
}

void TF_IDF_Indexer::decompress_posting(const Node_Compressed& node, std::vector<TF>& res) {
    size_t pos = 0;
    while (pos < node.postings.size()) {
        size_t norm_id = decode_vb(node.postings, pos);
        TF tf;
        tf.doc_id = ids[norm_id];
        tf.count = decode_vb(node.postings, pos);
        res.push_back(tf);
    }
}

void TF_IDF_Indexer::compress() {
    if (!sorted) {
        sort_index();
    }

    size_t counter = 1;
    for (auto& term : index) {
        std::cout << "Compressing term: " << counter++ << "\n";
        Node_Compressed n;
        n.lemma = term.lemma;
        for (auto& doc_id : term.doc_ids) {
            long mongo_pos = find_mongo_id(doc_id.doc_id);
            size_t norm_id = static_cast<size_t>(mongo_pos);
            if (mongo_pos == -1) {
                ids.push_back(doc_id.doc_id);
                norm_id = ids.size() - 1;
            }
            encode_vb(norm_id, n.postings);
            encode_vb(doc_id.count, n.postings);
        }
        index_compressed.push_back(n);
    }

    index.resize(0);
    compressed = true;
}
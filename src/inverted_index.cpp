#include "inverted_index.hpp"
#include "tokenizer.hpp"
#include <algorithm>

namespace fulltext_search_service {

    namespace {
        const std::vector<Entry> kEmptyPostings;
    } // namespace

    void InvertedIndex::UpdateDocumentBase(std::vector<std::string> input_docs) {
        docs_ = std::move(input_docs);
        freq_dictionary_.clear();

        for (size_t doc_id = 0; doc_id < docs_.size(); ++doc_id) {
            std::unordered_map<std::string, size_t> word_count;
            tokenize(docs_[doc_id], word_count);
            for (auto &[w, count]: word_count) {
                freq_dictionary_[std::move(w)].push_back({doc_id, count});
            }
        }

        for (auto &[word, list]: freq_dictionary_) {
            std::ranges::sort(list, {}, &Entry::doc_id);
        }
    }

    const std::vector<Entry> &InvertedIndex::GetWordCount(std::string_view word) const {
        auto it = freq_dictionary_.find(word);
        if (it == freq_dictionary_.end()) {
            return kEmptyPostings;
        }

        return it->second;
    }

    std::string InvertedIndex::GetDocument(size_t doc_id) const {
        if (doc_id >= docs_.size()) {
            return {};
        }

        return docs_[doc_id];
    }

} // namespace fulltext_search_service
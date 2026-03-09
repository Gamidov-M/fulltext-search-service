#include "tokenizer.hpp"
#include <ranges>

namespace fulltext_search_service {

    void tokenize(
            const std::string &text,
            std::unordered_map<std::string, size_t> &out,
            std::size_t max_word_length
    ) {
        out.reserve(out.size() + 32);
        for (auto word_range: text | std::views::split(' ')) {
            std::string word;
            word.reserve(32);
            for (char c: word_range) {
                word += c;
            }
            if (!word.empty() && word.size() <= max_word_length) {
                ++out[std::move(word)];
            }
        }
    }

} // namespace fulltext_search_service

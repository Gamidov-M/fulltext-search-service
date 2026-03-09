#pragma once

#include "types.hpp"
#include "inverted_index.hpp"
#include <cstddef>
#include <string>
#include <vector>

namespace fulltext_search_service {

    class Search {
    public:
        explicit Search(
                InvertedIndex &index,
                std::size_t max_word_length = 100
        ) : index_(index), max_word_length_(max_word_length) {}

        // Для каждого запроса до max_responses документов, отсортированных по убыванию ранга (нормализован в [0, 1])
        [[nodiscard]] std::vector<std::vector<RelativeIndex>> search(
                const std::vector<std::string> &queries,
                int max_responses
        ) const;

    private:
        InvertedIndex &index_;
        std::size_t max_word_length_;
    };

} // namespace fulltext_search_service

#pragma once

#include <cstddef>
#include <cmath>
#include <string>

namespace fulltext_search_service {

    // Один постинг в инвертированном индексе: doc_id и количество вхождений термина в документе
    struct Entry {
        size_t doc_id{};
        size_t count{};

        constexpr bool operator==(const Entry &other) const = default;
    };

    // Результат ранжирования по одному запросу: doc_id и нормализованный ранг в [0, 1]
    struct RelativeIndex {
        size_t doc_id{};
        float rank{};

        // Сравнение rank через epsilon из-за float
        [[nodiscard]] constexpr bool operator==(const RelativeIndex &other) const {
            return doc_id == other.doc_id && std::abs(rank - other.rank) < 1e-6f;
        }
    };

} // namespace fulltext_search_service

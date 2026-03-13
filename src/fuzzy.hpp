#pragma once

#include <functional>
#include <string>
#include <string_view>
#include <vector>

namespace fulltext_search_service {

    // Расстояние Левенштейна (редакционное) в символах (байтах для UTF-8)
    [[nodiscard]] int levenshtein_distance(std::string_view a, std::string_view b);

    // Пара: индексный термин и расстояние редактирования от запроса
    struct FuzzyMatch {
        std::string term;
        int distance{};
    };

    // Находит термины из словаря, близкие к word (расстояние Левенштейна <= max_edits).
    // iterate_terms(fn) должен вызвать fn(term) для каждого термина в словаре.
    // Проверяется длина (|len(word)-len(term)| <= max_edits), затем расстояние.
    // out сортируется по возрастанию расстояния; при равенстве - лексикографически.
    void find_similar_terms(
            std::string_view word,
            int max_edits,
            const std::function<void(const std::function<void(std::string_view)> &)> &iterate_terms,
            std::vector<FuzzyMatch> &out
    );

} // namespace fulltext_search_service

#pragma once

#include "types.hpp"
#include <nlohmann/json.hpp>
#include <string>
#include <unordered_set>

namespace fulltext_search_service {

    class Stemmer;

    // Собирает один сниппет: конкатенация текста из string-полей content (с подсветкой)
    // обрезанный до max_length символов с суффиксом suffix при обрезке
    // stemmer для нормализации слов
    [[nodiscard]] std::string buildSnippet(
            const nlohmann::json &content,
            const Collection &collection,
            const std::unordered_set<std::string> &terms,
            size_t max_length = 255,
            const std::string &suffix = "...",
            const std::string &pre = "<em>",
            const std::string &post = "</em>",
            const Stemmer *stemmer = nullptr
    );

    // Форматирование content для отображения: поля из crop_field_names обрезаются (crop_length, crop_marker)
    // при do_highlight к обрезанному тексту и к остальным string-полям применяется подсветка (pre, post)
    // stemmer для сопоставления слов с terms
    [[nodiscard]] nlohmann::json buildFormattedContent(
            const nlohmann::json &content,
            const Collection &collection,
            const std::unordered_set<std::string> &crop_field_names,
            const std::unordered_set<std::string> &terms,
            size_t crop_length,
            const std::string &crop_marker,
            bool do_highlight,
            const std::string &highlight_pre,
            const std::string &highlight_post,
            const Stemmer *stemmer = nullptr
    );

} // namespace fulltext_search_service

#pragma once

#include "types.hpp"
#include <nlohmann/json.hpp>
#include <string>
#include <unordered_set>

namespace fulltext_search_service {

    // Подсвечивает в строке целые слова из terms, оборачивая их в pre + слово + post
    [[nodiscard]] std::string highlightInString(
            const std::string &text,
            const std::unordered_set<std::string> &terms,
            const std::string &pre = "<em>",
            const std::string &post = "</em>"
    );

    // Возвращает копию content в которой все string-поля (по схеме collection)
    // заменены на версии с подсветкой терминов из terms
    [[nodiscard]] nlohmann::json highlightContent(
            const nlohmann::json &content,
            const Collection &collection,
            const std::unordered_set<std::string> &terms,
            const std::string &pre = "<em>",
            const std::string &post = "</em>"
    );

    // Собирает один сниппет: конкатенация текста из string-полей content (с подсветкой)
    // обрезанный до max_length символов с суффиксом suffix при обрезке
    [[nodiscard]] std::string buildSnippet(
            const nlohmann::json &content,
            const Collection &collection,
            const std::unordered_set<std::string> &terms,
            size_t max_length = 255,
            const std::string &suffix = "...",
            const std::string &pre = "<em>",
            const std::string &post = "</em>"
    );

} // namespace fulltext_search_service

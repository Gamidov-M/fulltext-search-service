#pragma once

#include "types.hpp"
#include <nlohmann/json.hpp>
#include <string>
#include <unordered_set>

namespace fulltext_search_service {

    class Stemmer;

    // Подсвечивает в строке целые слова из terms, оборачивая их в pre + слово + post
    // Если stemmer != nullptr, при проверке вхождения слово нормализуется (для совпадения с индексом)
    [[nodiscard]] std::string highlightInString(
            const std::string &text,
            const std::unordered_set<std::string> &terms,
            const std::string &pre = "<em>",
            const std::string &post = "</em>",
            const Stemmer *stemmer = nullptr
    );

    // Возвращает копию content в которой все string-поля (по схеме collection)
    // заменены на версии с подсветкой терминов из terms
    // stemmer для нормализации слов при проверке
    [[nodiscard]] nlohmann::json highlightContent(
            const nlohmann::json &content,
            const Collection &collection,
            const std::unordered_set<std::string> &terms,
            const std::string &pre = "<em>",
            const std::string &post = "</em>",
            const Stemmer *stemmer = nullptr
    );

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

    // Обрезает строку до crop_length слов вокруг первого совпадения с terms
    // В местах обрезки подставляется crop_marker (например "...")
    [[nodiscard]] std::string cropField(
            const std::string &text,
            const std::unordered_set<std::string> &terms,
            size_t crop_length,
            const std::string &crop_marker,
            const Stemmer *stemmer = nullptr
    );

    // Возвращает JSON с полями из crop_field_names, значения которых обрезанные строки
    // Только string-поля из collection, входящие в crop_field_names, обрабатываются
    [[nodiscard]] nlohmann::json cropContent(
            const nlohmann::json &content,
            const Collection &collection,
            const std::unordered_set<std::string> &crop_field_names,
            const std::unordered_set<std::string> &terms,
            size_t crop_length,
            const std::string &crop_marker,
            const Stemmer *stemmer = nullptr
    );

} // namespace fulltext_search_service

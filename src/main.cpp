#include "inverted_index.hpp"
#include <exception>
#include <print>

int main() {
    try {
        using namespace fulltext_search_service;
        InvertedIndex index;
        index.SetStoragePath("./index-data");
        if (!index.Load()) {
            std::println(stderr, "Не удалось загрузить индекс.");
        } else {
            std::println("Индекс загружен, документов: {}", index.GetDocumentCount());
        }

        index.UpdateDocumentBase({
             "тест документ с текстом",
             "тест второй документ",
             "мда текст и еще текст"
        });

        std::println("Документов: {}", index.GetDocumentCount());

        for (const auto &e: index.GetWordCount("документ")) {
            std::println("  doc-id={} count={}", e.doc_id, e.count);
        }

    } catch (const std::exception &ex) {
        std::println(stderr, "Ошибка: {}", ex.what());
        return 1;
    }

    return 0;
}
#include "inverted_index.hpp"
#include <print>

int main() {
    using namespace fulltext_search_service;
    InvertedIndex index;
    index.UpdateDocumentBase({
        "тест документ с текстом",
        "тест второй документ",
        "мда текст и еще текст"
    });

    std::println("Документов: {}", index.GetDocumentCount());

    for (const auto &e: index.GetWordCount("документ")) {
        std::println("  doc_id={} count={}", e.doc_id, e.count);
    }

    return 0;
}
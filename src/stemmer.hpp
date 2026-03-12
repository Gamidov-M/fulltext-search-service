#pragma once

#include <memory>
#include <string>

namespace fulltext_search_service {

    // Обёртка над Snowball (libstemmer) - нормализация словоформ (стемминг) для индексации и запросов
    // Потокобезопасна (внутренняя блокировка)
    class Stemmer {
    public:
        // Создаёт стеммер для алгоритма и кодировки (nullptr = UTF-8)
        // Если алгоритм не найден или ошибка - возвращает nullptr
        [[nodiscard]] static std::unique_ptr<Stemmer> create(const char *algorithm, const char *charenc = nullptr);

        Stemmer(const Stemmer &) = delete;
        Stemmer &operator=(const Stemmer &) = delete;
        ~Stemmer();

        // Возвращает нормализованную (стеммированную) форму слова
        // UTF-8
        [[nodiscard]] std::string normalize(const std::string &word) const;

    private:
        struct Impl;
        explicit Stemmer(std::unique_ptr<Impl> impl);
        std::unique_ptr<Impl> impl_;
    };

} // namespace fulltext_search_service

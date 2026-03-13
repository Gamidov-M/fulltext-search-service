#pragma once

#include <cstddef>
#include <string>
#include <unordered_map>
#include <vector>

namespace fulltext_search_service {

    class Stemmer;

    // Разбивает текст на слова по пробелам и заполняет карту 'слово -> число вхождений'
    // out не очищается, только дополняется.
    // Слова длиннее max_word_length отбрасываются (max_word_length задаётся из конфига: index.max_word_length)
    // Если stemmer не nullptr, каждое слово нормализуется (стеммируется) перед добавлением в out
    void tokenize(
            const std::string &text,
            std::unordered_map<std::string, size_t> &out,
            std::size_t max_word_length,
            const Stemmer *stemmer = nullptr
    );

    // Разбивает текст на слова по пробелам и заполняет вектор терминов в порядке появления
    // Используется для фразового поиска (проверка точной последовательности терминов)
    void tokenizeToSequence(
            const std::string &text,
            std::vector<std::string> &out,
            std::size_t max_word_length,
            const Stemmer *stemmer = nullptr
    );

} // namespace fulltext_search_service

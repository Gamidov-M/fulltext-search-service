#pragma once

#include <cstddef>
#include <string>
#include <unordered_map>

namespace fulltext_search_service {

    // Разбивает текст на слова по пробелам и заполняет карту 'слово -> число вхождений'
    // out не очищается, только дополняется.
    // Слова длиннее max_word_length отбрасываются (max_word_length задаётся из конфига: index.max_word_length)
    void tokenize(
            const std::string &text,
            std::unordered_map <std::string,
            size_t> &out,
            std::size_t max_word_length
    );

} // namespace fulltext_search_service

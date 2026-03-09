#pragma once

#include <cstddef>
#include <string>
#include <unordered_map>

namespace fulltext_search_service {

    // Максимальная длина слова; более длинные токены отбрасываются при индексации и при разборе запроса
    constexpr std::size_t kMaxWordLength = 100;

    // Разбивает текст на слова по пробелам и заполняет карту 'слово -> число вхождений'
    // out не очищается, только дополняется
    void tokenize(const std::string &text, std::unordered_map<std::string, size_t> &out);

} // namespace fulltext_search_service

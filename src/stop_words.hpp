#pragma once

#include "config.hpp"
#include <memory>
#include <string>
#include <unordered_set>

namespace fulltext_search_service {

    // Собирает множество стоп-слов в том же виде, что и токены после ToLowerUtf8 и стемминга (если стемминг включён в index)
    // Пустой файл и пустой список в конфиге -> nullptr
    [[nodiscard]] std::shared_ptr<const std::unordered_set<std::string>> LoadStopWordsSet(
            const IndexConfig &index,
            std::string_view config_file_path,
            bool dev_mode
    );

} // namespace fulltext_search_service

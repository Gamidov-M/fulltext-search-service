#include "api_server.hpp"
#include "config.hpp"
#include "inverted_index.hpp"
#include <exception>
#include <print>
#include <string_view>

namespace {

    // Возвращает путь к конфигу при наличии --config=<путь>, иначе nullopt
    std::optional <std::string> get_config_path(int argc, char *argv[]) {
        for (int i = 1; i < argc; ++i) {
            std::string_view arg(argv[i]);
            if (arg.starts_with("--config=")) {
                std::string path(arg.substr(9));
                return path.empty() ? std::optional<std::string>("") : std::optional(path);
            }
        }

        return std::nullopt;
    }

} // namespace

int main(int argc, char *argv[]) {
    try {
        using namespace fulltext_search_service;

        AppConfig config;
        constexpr const char *kDefaultConfigPath = "/etc/fulltext-search-service/config.yaml";

        if (auto path_opt = get_config_path(argc, argv)) {
            if (path_opt->empty()) {
                std::println(stderr, "Укажите путь к конфиг-файлу: --config=<файл>");
                return 1;
            }
            if (auto loaded = LoadConfig(*path_opt)) {
                config = std::move(*loaded);
            } else {
                std::println(stderr, "Не удалось загрузить конфиг из: {}", *path_opt);
                return 1;
            }
        } else {
            if (auto loaded = LoadConfig(kDefaultConfigPath)) {
                config = std::move(*loaded);
            } else {
                std::println(stderr, "Не удалось загрузить конфиг из: {}", kDefaultConfigPath);
                return 1;
            }
        }

        InvertedIndex index;
        index.SetStoragePath(config.index.storage_path);
        index.SetMaxWordLength(config.index.max_word_length);
        if (!index.Load()) {
            std::println(stderr, "Не удалось загрузить индекс.");
        }

        ApiServer api(index, config.api, config.server, config.index);
        if (!api.listen(config.server.host, config.server.port)) {
            std::println(stderr, "Не удалось запустить http сервер.");
            return 1;
        }
    } catch (const std::exception &ex) {
        std::println(stderr, "Ошибка: {}", ex.what());
        return 1;
    }

    return 0;
}

#include <httplib.h>
#include <nlohmann/json.hpp>
#include <print>

int main() {
    using json = nlohmann::json;

    httplib::Client cli("127.0.0.1", 8000);
    cli.set_connection_timeout(2, 0);

    auto get_json = [&cli](const httplib::Result &res) -> json {
        if (!res || res->status >= 400) {
            return nullptr;
        }

        return json::parse(res->body.empty() ? "{}" : res->body);
    };

    std::println("Загрузка документов");

    json docs_body = json::array(
            {
                    {{"content", "тест документ с текстом"}},
                    {{"content", "тест второй документ"}},
                    {{"content", "мда текст и еще текст"}},
            }
    );

    auto post_docs = cli.Post("/indexes/documents", docs_body.dump(), "application/json");
    json docs_res = get_json(post_docs);
    if (!docs_res.is_null()) {
        std::println(
                "Ответ: received={}, indexUid={}\n",
                docs_res.value("received", 0), docs_res.value("indexUid", "")
        );
    } else {
        std::println("Ошибка загрузки документов");
    }

    std::println("Список документов");

    auto get_docs = cli.Get("/indexes/documents?offset=0&limit=10");
    json list_res = get_json(get_docs);
    if (!list_res.is_null() && list_res.contains("results")) {
        for (const auto &item: list_res["results"]) {
            std::println("  id={} content=\"{}\"", item.value("id", -1), item.value("content", ""));
        }

        std::println("  total={}\n", list_res.value("total", 0));
    } else {
        std::println("Ошибка получения списка документов");
    }

    std::println("Поиск");

    for (const auto &query: {"тест", "второй документ", "мда текст"}) {
        json search_body = {
                {"q",     query},
                {"limit", 5}
        };

        auto post_search = cli.Post("/indexes/search", search_body.dump(), "application/json");
        json search_res = get_json(post_search);

        if (!search_res.is_null() && search_res.contains("hits")) {
            std::println("Запрос: \"{}\"", query);

            for (const auto &hit: search_res["hits"]) {
                int id = hit.value("id", -1);
                std::string content = hit.value("content", "");
                double score = hit.value("_rankingScore", 0.0);
                std::println("  id={} _rankingScore={:.4f} \"{}\"", id, score, content);
            }

            std::println("");
        } else {
            std::println("Ошибка поиска по запросу \"{}\"", query);
        }
    }

    return 0;
}

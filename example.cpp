#include <httplib.h>
#include <nlohmann/json.hpp>
#include <print>

int main() {
    using json = nlohmann::json;

    httplib::Client cli("127.0.0.1", 8000);
    cli.set_connection_timeout(2, 0);

    auto get_json = [](const httplib::Result &res) -> json {
        if (!res || res->status >= 400) {
            return nullptr;
        }
        return json::parse(res->body.empty() ? "{}" : res->body);
    };

    std::println("1 -- Создание схемы (id: int, name: string)");

    json scheme_body = {
        {"fields", json::array({
            {{"name", "id"},  {"type", "int"}},
            {{"name", "name"}, {"type", "string"}}
        })}
    };
    auto post_scheme = cli.Post("/indexes/schemes", scheme_body.dump(), "application/json");
    json scheme_res = get_json(post_scheme);
    if (scheme_res.is_null() || !scheme_res.contains("fields")) {
        std::println("Ошибка создания схемы\n");
    } else {
        std::println("Схема создана\n");
    }

    std::println("2 -- Загрузка документов");

    json docs_body = json::array(
            {
                {{"content", {{"id", 1}, {"name", "тест документ с текстом"}}}},
                {{"content", {{"id", 2}, {"name", "тест второй документ"}}}},
                {{"content", {{"id", 3}, {"name", "мда текст и еще текст"}}}},
            }
    );

    auto post_docs = cli.Post("/indexes/documents", docs_body.dump(), "application/json");
    json docs_res = get_json(post_docs);
    if (!docs_res.is_null()) {
        std::println("Ответ: received={}\n", docs_res.value("received", 0));
    } else {
        std::println("Ошибка загрузки документов");
        return 1;
    }

    std::println("3 -- Список документов");

    auto get_docs = cli.Get("/indexes/documents?offset=0&limit=10");
    json list_res = get_json(get_docs);
    if (!list_res.is_null() && list_res.contains("results")) {
        for (const auto &item: list_res["results"]) {
            int doc_id = item.value("id", -1);
            const json &content = item.contains("content") ? item["content"] : json::object();
            std::println("  id={} content={}", doc_id, content.dump());
        }

        std::println("  total={}\n", list_res.value("total", 0));
    } else {
        std::println("Ошибка получения списка документов");
    }

    std::println("4 -- Поиск");

    for (const auto &query: {"тест", "второй документ", "мда текст"}) {
        json search_body = {{"q", query}, {"limit", 5}};
        auto post_search = cli.Post("/indexes/search", search_body.dump(), "application/json");
        json search_res = get_json(post_search);

        if (!search_res.is_null() && search_res.contains("results")) {
            std::println("Запрос: \"{}\"", query);
            for (const auto &hit: search_res["results"]) {
                int id = hit.value("id", -1);
                const json &content = hit.contains("content") ? hit["content"] : json::object();
                double score = hit.value("_rankingScore", 0.0);
                std::println("  id={} _rankingScore={:.4f} content={}", id, score, content.dump());
            }

            std::println("");
        } else {
            std::println("Ошибка поиска по запросу \"{}\"", query);
        }
    }

    return 0;
}

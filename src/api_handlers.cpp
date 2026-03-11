#include "api_handlers.hpp"
#include "utils.hpp"
#include <chrono>
#include <nlohmann/json.hpp>

namespace fulltext_search_service {

    void handleSearch(
            InvertedIndex &index,
            Search &search,
            const ApiConfigSection &api,
            const httplib::Request &req,
            httplib::Response &res,
            bool dev_mode
    ) {
        nlohmann::json body;
        try {
            body = nlohmann::json::parse(req.body.empty() ? "{}" : req.body);
        } catch (const nlohmann::json::exception &) {
            Log(dev_mode, "[dev] search: неверный json");
            sendJson(res, 400, {
                    {"message", "Некорректный JSON"},
                    {"code",    "invalid_request"}
            });
            return;
        }

        std::string query = body.value("q", "");
        int limit = std::clamp(body.value("limit", api.max_responses), 1, api.max_limit);
        int offset = std::clamp(body.value("offset", 0), 0, api.max_offset);

        const int request_size = std::min(
                offset + limit,
                api.max_offset + api.max_limit
        );

        auto start = std::chrono::steady_clock::now();
        auto results = search.search(std::vector{query}, request_size);
        auto processing_time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - start
        ).count();

        const auto &full_list = results.empty() ? std::vector<RelativeIndex>{} : results[0];
        const size_t from = static_cast<size_t>(offset);
        const size_t to = std::min(from + static_cast<size_t>(limit), full_list.size());

        nlohmann::json results_json = nlohmann::json::array();
        for (size_t i = from; i < to; ++i) {
            const auto &rel = full_list[i];
            results_json.push_back(
                    {
                            {"id",            static_cast<int>(rel.doc_id)},
                            {"content",       index.GetDocument(rel.doc_id)},
                            {"_rankingScore", rel.rank}
                    }
            );
        }
        Log(dev_mode, "[dev] search q=\"{}\" results={}", query, full_list.size());
        sendJson(res, 200, {
                {"results",          results_json},
                {"offset",           offset},
                {"limit",            limit},
                {"total",            full_list.size()},
                {"processingTimeMs", processing_time_ms},
                {"query",            query}
        });
    }

    void handleGetDocuments(
            InvertedIndex &index,
            const ApiConfigSection &api,
            const httplib::Request &req,
            httplib::Response &res,
            bool dev_mode
    ) {
        const size_t total = index.GetDocumentCount();
        const int offset = parseQueryInt(req, "offset", 0, 0, api.max_offset);
        const int limit = parseQueryInt(req, "limit", api.default_limit, 1, api.max_limit);
        nlohmann::json results = nlohmann::json::array();
        for (size_t i = static_cast<size_t>(offset), n = 0; i < total && n < static_cast<size_t>(limit); ++i, ++n) {
            results.push_back(
                    {
                            {"id",      static_cast<int>(i)},
                            {"content", index.GetDocument(i)}
                    }
            );
        }
        Log(dev_mode, "[dev] docs offset={} limit={} total={}", offset, limit, total);
        sendJson(res, 200, {
                {"results", results},
                {"offset",  offset},
                {"limit",   limit},
                {"total",   static_cast<int>(total)}
        });
    }

    void handlePostDocuments(
            InvertedIndex &index,
            const httplib::Request &req,
            httplib::Response &res,
            bool dev_mode
    ) {
        if (req.body.empty()) {
            Log(dev_mode, "[dev] post: пустое тело");
            sendJson(res, 400, {
                    {"message", "Тело запроса пусто"},
                    {"code",    "invalid_request"}
            });
            return;
        }

        nlohmann::json body;
        try {
            body = nlohmann::json::parse(req.body);
        } catch (const nlohmann::json::exception &) {
            Log(dev_mode, "[dev] post: неверный json");
            sendJson(res, 400, {
                    {"message", "Некорректный JSON"},
                    {"code",    "invalid_request"}
            });
            return;
        }

        if (!body.is_array()) {
            Log(dev_mode, "[dev] post: не массив");
            sendJson(res, 400, {
                    {"message", "JSON должно быть массивом документов"},
                    {"code",    "invalid_request"}
            });
            return;
        }

        std::vector<InvertedIndex::DocumentInput> documents;
        documents.reserve(body.size());
        if (!index.HasSchema()) {
            Log(dev_mode, "[dev] post: схема не задана");
            sendJson(res, 400, {
                    {"message", "Создайте схему"},
                    {"code",    "schema_required"}
            });
            return;
        }

        const Schema &schema = index.GetSchema();
        for (auto &item: body) {
            if (!item.is_object()) {
                Log(dev_mode, "[dev] post: элемент не объект");
                sendJson(res, 400, {
                        {"message", "Каждый документ должен быть json объектом с полем content"},
                        {"code",    "invalid_request"}
                });
                return;
            }

            auto it = item.find("content");
            if (it == item.end() || !it->is_object()) {
                Log(dev_mode, "[dev] post: нет content или content не объект");
                sendJson(res, 400, {
                        {"message", "У документа должно быть поле content (объект по схеме)"},
                        {"code",    "invalid_request"}
                });
                return;
            }

            nlohmann::json content = *it;
            for (const auto &field: schema.fields) {
                auto f = content.find(field.name);
                if (f == content.end()) {
                    sendJson(res, 400, {
                            {"message", "В документе отсутствует поле: " + field.name},
                            {"code",    "invalid_request"}
                    });
                    return;
                }

                if (field.type == "int") {
                    if (!f->is_number_integer() && !f->is_number_unsigned()) {
                        sendJson(res, 400, {
                                {"message", "Поле " + field.name + " должно быть типа int"},
                                {"code",    "invalid_request"}
                        });
                        return;
                    }
                } else if (field.type == "string") {
                    if (!f->is_string()) {
                        sendJson(res, 400, {
                                {"message", "Поле " + field.name + " должно быть типа string"},
                                {"code",    "invalid_request"}
                        });
                        return;
                    }
                }
            }
            documents.push_back({std::move(content)});
        }

        const int received = static_cast<int>(documents.size());
        Log(dev_mode, "[dev] post received={}", received);
        index.UpdateDocumentBase(std::move(documents));
        sendJson(res, 202, {
                {"received", received}
        });
    }

    void handleGetScheme(
            InvertedIndex &index,
            const httplib::Request &,
            httplib::Response &res,
            bool dev_mode
    ) {
        if (!index.HasSchema()) {
            Log(dev_mode, "[dev] get scheme: схема не задана");
            sendJson(res, 200, {
                    {"fields", nlohmann::json::array()}
            });
            return;
        }

        nlohmann::json fields = nlohmann::json::array();
        for (const auto &f: index.GetSchema().fields) {
            fields.push_back(
                    {
                            {"name", f.name},
                            {"type", f.type}
                    }
            );
        }

        sendJson(res, 200, {{"fields", fields}});
    }

    void handlePostScheme(
            InvertedIndex &index,
            const httplib::Request &req,
            httplib::Response &res,
            bool dev_mode
    ) {
        if (req.body.empty()) {
            sendJson(res, 400, {
                    {"message", "Тело запроса пусто"},
                    {"code",    "invalid_request"}
            });
            return;
        }

        nlohmann::json body;
        try {
            body = nlohmann::json::parse(req.body);
        } catch (const nlohmann::json::exception &) {
            sendJson(res, 400, {
                    {"message", "Некорректный JSON"},
                    {"code",    "invalid_request"}
            });
            return;
        }

        auto it = body.find("fields");
        if (it == body.end() || !it->is_array()) {
            sendJson(res, 400, {
                    {"message", "Ожидается объект с полем fields (массив полей)"},
                    {"code",    "invalid_request"}
            });
            return;
        }

        Schema schema;
        for (auto &el: *it) {
            if (!el.is_object()) {
                sendJson(res, 400, {
                        {"message", "Каждый элемент fields должен быть объектом с name и type"},
                        {"code",    "invalid_request"}
                });
                return;
            }

            std::string name = el.value("name", "");
            std::string type = el.value("type", "");
            if (name.empty() || (type != "int" && type != "string")) {
                sendJson(res, 400, {
                        {"message", "Поле должно иметь name (строка) и type (int или string)"},
                        {"code",    "invalid_request"}
                });
                return;
            }
            schema.fields.push_back({std::move(name), std::move(type)});
        }

        index.SetSchema(std::move(schema));
        if (!index.SaveSchema()) {
            sendJson(res, 500, {
                    {"message", "Не удалось сохранить схему"},
                    {"code",    "internal_error"}
            });
            return;
        }

        nlohmann::json fields = nlohmann::json::array();
        for (const auto &f: index.GetSchema().fields) {
            fields.push_back(
                    {
                            {"name", f.name},
                            {"type", f.type}
                    }
            );
        }

        Log(dev_mode, "[dev] создана схема, fields={}", index.GetSchema().fields.size());
        sendJson(res, 201, {{"fields", fields}});
    }

    void handleDeleteScheme(
            InvertedIndex &index,
            const httplib::Request &,
            httplib::Response &res,
            bool dev_mode
    ) {
        index.SetSchema(Schema{});
        index.SaveSchema();
        Log(dev_mode, "[dev] схема удалена");
        sendJson(res, 200, {{"message", "Схема удалена"}});
    }

} // namespace fulltext_search_service

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

        nlohmann::json hits = nlohmann::json::array();
        for (size_t i = from; i < to; ++i) {
            const auto &rel = full_list[i];
            hits.push_back(
                    {
                            {"id",            static_cast<int>(rel.doc_id)},
                            {"content",       index.GetDocument(rel.doc_id)},
                            {"_rankingScore", rel.rank}
                    }
            );
        }
        Log(dev_mode, "[dev] search q=\"{}\" hits={}", query, full_list.size());
        sendJson(res, 200, {
                {"hits",               hits},
                {"offset",             offset},
                {"limit",              limit},
                {"estimatedTotalHits", full_list.size()},
                {"processingTimeMs",   processing_time_ms},
                {"query",              query}
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

        std::vector<std::string> documents;
        documents.reserve(body.size());
        for (auto &item: body) {
            if (!item.is_object()) {
                Log(dev_mode, "[dev] post: элемент не объект");
                sendJson(res, 400, {
                        {"message", "Каждый документ должен быть JSON-объектом с полем content"},
                        {"code",    "invalid_request"}
                });
                return;
            }

            auto it = item.find("content");
            if (it == item.end() || !it->is_string()) {
                Log(dev_mode, "[dev] post: нет content");
                sendJson(res, 400, {
                        {"message", "У каждого документа должно быть строковое поле content"},
                        {"code",    "invalid_request"}
                });
                return;
            }
            documents.push_back(it->get<std::string>());
        }

        const int received = static_cast<int>(documents.size());
        Log(dev_mode, "[dev] post received={}", received);
        index.UpdateDocumentBase(std::move(documents));
        sendJson(res, 202, {
                {"received", received}
        });
    }

} // namespace fulltext_search_service

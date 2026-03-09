#include "api_server.hpp"
#include "api_handlers.hpp"
#include "response_utils.hpp"
#include <print>

// Размер пула потоков cpp-httplib
// число одновременных запросов без блокировки
#ifndef CPPHTTPLIB_THREAD_POOL_COUNT
#define CPPHTTPLIB_THREAD_POOL_COUNT 64
#endif

#include <httplib.h>

namespace fulltext_search_service {

    ApiServer::ApiServer(
            InvertedIndex &index,
            const ApiConfigSection &api_config,
            const ServerConfig &server_config,
            const IndexConfig &index_config
    ) : index_(index), api_config_(api_config), server_config_(server_config),
        index_config_(index_config),
        search_(std::make_unique<Search>(index, static_cast<std::size_t>(index_config.max_word_length))),
        server_(std::make_unique<httplib::Server>()) {
        server_->set_keep_alive_max_count(server_config_.keep_alive_max_count);
        setupRoutes();
    }

    ApiServer::~ApiServer() = default;

    void ApiServer::setupRoutes() {
        server_->Post("/indexes/search", [this](const httplib::Request &req, httplib::Response &res) {
            handleSearch(index_, *search_, api_config_, req, res);
        });
        server_->Get("/indexes/documents", [this](const httplib::Request &req, httplib::Response &res) {
            handleGetDocuments(index_, api_config_, req, res);
        });
        server_->Post("/indexes/documents", [this](const httplib::Request &req, httplib::Response &res) {
            handlePostDocuments(index_, req, res);
        });
        server_->set_error_handler([](const httplib::Request &, httplib::Response &res) {
            if (res.status == 404) {
                sendJson(res, 404, {
                        {"message", "Не найдено"},
                        {"code",    "not_found"}
                });
            }
        });
    }

    bool ApiServer::listen(const std::string &host, int port) {
        std::println("{}:{}", host, port);
        return server_->listen(host, port);
    }

    void ApiServer::stop() {
        if (server_) {
            server_->stop();
        }
    }

} // namespace fulltext_search_service

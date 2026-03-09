#pragma once

#include "config.hpp"
#include "inverted_index.hpp"
#include "search.hpp"
#include <httplib.h>

namespace fulltext_search_service {

    void handleSearch(
            InvertedIndex &index,
            Search &search,
            const ApiConfigSection &api,
            const httplib::Request &req,
            httplib::Response &res
    );

    void handleGetDocuments(
            InvertedIndex &index,
            const ApiConfigSection &api,
            const httplib::Request &req,
            httplib::Response &res
    );

    void handlePostDocuments(InvertedIndex &index, const httplib::Request &req, httplib::Response &res);

} // namespace fulltext_search_service

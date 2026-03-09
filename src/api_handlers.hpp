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
            httplib::Response &res,
            bool dev_mode = false
    );

    void handleGetDocuments(
            InvertedIndex &index,
            const ApiConfigSection &api,
            const httplib::Request &req,
            httplib::Response &res,
            bool dev_mode = false
    );

    void handlePostDocuments(
            InvertedIndex &index,
            const httplib::Request &req,
            httplib::Response &res,
            bool dev_mode = false
    );

} // namespace fulltext_search_service

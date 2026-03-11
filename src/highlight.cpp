#include "highlight.hpp"
#include <sstream>

namespace fulltext_search_service {

    std::string highlightInString(
            const std::string &text,
            const std::unordered_set <std::string> &terms,
            const std::string &pre,
            const std::string &post
    ) {
        if (text.empty()) {
            return text;
        }
        std::string result;
        result.reserve(text.size() + terms.size() * (pre.size() + post.size() + 16));

        for (size_t i = 0; i < text.size();) {
            while (i < text.size() && text[i] == ' ') {
                result += text[i++];
            }

            if (i >= text.size()) {
                break;
            }

            const size_t start = i;
            while (i < text.size() && text[i] != ' ') {
                ++i;
            }

            std::string word(text.substr(start, i - start));
            if (terms.count(word)) {
                result += pre;
                result += word;
                result += post;
            } else {
                result += word;
            }
        }

        return result;
    }

    nlohmann::json highlightContent(
            const nlohmann::json &content,
            const Collection &collection,
            const std::unordered_set <std::string> &terms,
            const std::string &pre,
            const std::string &post
    ) {
        if (!content.is_object() || terms.empty()) {
            return content;
        }

        nlohmann::json out = content;
        for (const auto &field: collection.fields) {
            if (field.type != "string") {
                continue;
            }

            auto it = out.find(field.name);
            if (it == out.end() || !it->is_string()) {
                continue;
            }

            std::string value = it->get<std::string>();
            out[field.name] = highlightInString(value, terms, pre, post);
        }
        return out;
    }

    std::string buildSnippet(
            const nlohmann::json &content,
            const Collection &collection,
            const std::unordered_set <std::string> &terms,
            size_t max_length,
            const std::string &suffix,
            const std::string &pre,
            const std::string &post
    ) {
        if (!content.is_object()) {
            return {};
        }

        std::ostringstream combined;
        bool first = true;
        for (const auto &field: collection.fields) {
            if (field.type != "string") {
                continue;
            }

            auto it = content.find(field.name);
            if (it == content.end() || !it->is_string()) {
                continue;
            }

            const std::string &value = it->get_ref<const std::string &>();
            if (value.empty()) {
                continue;
            }

            if (!first) {
                combined << ' ';
            }

            first = false;
            combined << highlightInString(value, terms, pre, post);
        }

        std::string full = combined.str();
        if (full.size() <= max_length) {
            return full;
        }
        return full.substr(0, max_length) + suffix;
    }

} // namespace fulltext_search_service

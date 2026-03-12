#include "highlight.hpp"
#include "stemmer.hpp"
#include "utils.hpp"
#include <sstream>
#include <vector>

namespace fulltext_search_service {

    namespace {

    std::string highlightInString(
            const std::string &text,
            const std::unordered_set <std::string> &terms,
            const std::string &pre,
            const std::string &post,
            const Stemmer *stemmer
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
            std::string key;
            if (stemmer) {
                std::string word_lower = word;
                ToLowerUtf8(word_lower);
                key = stemmer->normalize(word_lower);
            } else {
                key = word;
                ToLowerUtf8(key);
            }
            if (!key.empty() && terms.count(key)) {
                result += pre;
                result += word;
                result += post;
            } else {
                result += word;
            }
        }

        return result;
    }

    bool wordMatches(
            const std::string &word,
            const std::unordered_set<std::string> &terms,
            const Stemmer *stemmer,
            std::string &key_out
    ) {
        if (stemmer) {
            std::string word_lower = word;
            ToLowerUtf8(word_lower);
            key_out = stemmer->normalize(word_lower);
        } else {
            key_out = word;
            ToLowerUtf8(key_out);
        }

        return !key_out.empty() && terms.count(key_out);
    }

    std::string cropField(
            const std::string &text,
            const std::unordered_set<std::string> &terms,
            size_t crop_length,
            const std::string &crop_marker,
            const Stemmer *stemmer
    ) {
        if (text.empty() || terms.empty() || crop_length == 0) {
            return text;
        }

        std::vector<std::string> words;
        words.reserve(64);
        for (size_t i = 0; i < text.size();) {
            while (i < text.size() && text[i] == ' ') {
                ++i;
            }

            if (i >= text.size()) {
                break;
            }

            const size_t start = i;
            while (i < text.size() && text[i] != ' ') {
                ++i;
            }
            words.push_back(std::string(text.substr(start, i - start)));
        }

        if (words.empty()) {
            return text;
        }

        size_t match_idx = words.size();
        std::string key;
        for (size_t j = 0; j < words.size(); ++j) {
            if (wordMatches(words[j], terms, stemmer, key)) {
                match_idx = j;
                break;
            }
        }

        if (match_idx >= words.size()) {
            return text;
        }

        const size_t half = crop_length / 2;
        size_t start_idx = (match_idx >= half) ? (match_idx - half) : 0;
        size_t end_idx = start_idx + crop_length;
        if (end_idx > words.size()) {
            end_idx = words.size();
            if (end_idx > start_idx + crop_length) {
                start_idx = (end_idx > crop_length) ? (end_idx - crop_length) : 0;
            }
        }

        std::string result;
        const bool cut_start = (start_idx > 0);
        const bool cut_end = (end_idx < words.size());
        if (cut_start) {
            result += crop_marker;
            result += ' ';
        }

        for (size_t k = start_idx; k < end_idx; ++k) {
            if (k > start_idx) {
                result += ' ';
            }
            result += words[k];
        }

        if (cut_end) {
            result += ' ';
            result += crop_marker;
        }

        return result;
    }

    } // namespace

    std::string buildSnippet(
            const nlohmann::json &content,
            const Collection &collection,
            const std::unordered_set <std::string> &terms,
            size_t max_length,
            const std::string &suffix,
            const std::string &pre,
            const std::string &post,
            const Stemmer *stemmer
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
            combined << highlightInString(value, terms, pre, post, stemmer);
        }

        std::string full = combined.str();
        if (full.size() <= max_length) {
            return full;
        }
        return full.substr(0, max_length) + suffix;
    }

    nlohmann::json buildFormattedContent(
            const nlohmann::json &content,
            const Collection &collection,
            const std::unordered_set<std::string> &crop_field_names,
            const std::unordered_set<std::string> &terms,
            size_t crop_length,
            const std::string &crop_marker,
            bool do_highlight,
            const std::string &highlight_pre,
            const std::string &highlight_post,
            const Stemmer *stemmer
    ) {
        nlohmann::json out = nlohmann::json::object();
        if (!content.is_object() || terms.empty()) {
            return out;
        }

        for (const auto &field : collection.fields) {
            if (field.type != "string") {
                continue;
            }
            auto it = content.find(field.name);
            if (it == content.end() || !it->is_string()) {
                continue;
            }
            const std::string &value = it->get_ref<const std::string &>();

            if (crop_field_names.count(field.name) != 0) {
                std::string s = cropField(value, terms, crop_length, crop_marker, stemmer);
                if (do_highlight) {
                    s = highlightInString(s, terms, highlight_pre, highlight_post, stemmer);
                }
                out[field.name] = std::move(s);
            } else if (do_highlight) {
                out[field.name] = highlightInString(value, terms, highlight_pre, highlight_post, stemmer);
            }
        }
        return out;
    }

} // namespace fulltext_search_service

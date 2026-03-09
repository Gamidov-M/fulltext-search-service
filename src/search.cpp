#include "search.hpp"
#include "tokenizer.hpp"
#include <algorithm>
#include <mutex>
#include <ranges>
#include <thread>
#include <unordered_map>

namespace fulltext_search_service {

    namespace {

        // tokenize(query) -> суммирование релевантности по doc_id -> нормализация ранга [0, 1] -> сортировка -> топ max_responses
        void process_one_query(
                const InvertedIndex &index,
                const std::string &query,
                int max_responses,
                std::size_t max_word_length,
                std::vector <RelativeIndex> &out
        ) {
            std::unordered_map <std::string, size_t> word_count;
            tokenize(query, word_count, max_word_length);

            std::unordered_map <size_t, size_t> doc_relevance;
            doc_relevance.reserve(256);
            for (const auto &[word, _]: word_count) {
                for (const auto &entry: index.GetWordCount(word)) {
                    doc_relevance[entry.doc_id] += entry.count;
                }
            }

            if (doc_relevance.empty()) {
                out.clear();
                return;
            }

            // Ранг нормализуем относительно максимума по текущему ответу (не по всему индексу)
            const size_t max_rel = std::ranges::max_element(doc_relevance, {}, [](const auto &p) {
                return p.second;
            })->second;
            const float max_rel_f = static_cast<float>(max_rel);
            out.clear();
            out.reserve(doc_relevance.size());
            for (const auto &[doc_id, count]: doc_relevance) {
                out.push_back({doc_id, static_cast<float>(count) / max_rel_f});
            }

            // Сначала по убыванию ранга
            // при равенстве - по doc_id для стабильного порядка
            std::ranges::sort(out, [](const auto &a, const auto &b) {
                if (a.rank != b.rank) {
                    return a.rank > b.rank;
                }
                return a.doc_id < b.doc_id;
            });

            if (out.size() > static_cast<size_t>(max_responses)) {
                out.resize(static_cast<size_t>(max_responses));
            }
        }

    } // namespace

    std::vector <std::vector<RelativeIndex>> Search::search(
            const std::vector <std::string> &queries,
            int max_responses
    ) const {
        std::vector <std::vector<RelativeIndex>> results(queries.size());
        if (queries.empty()) {
            return results;
        }

        const unsigned num_workers = std::min(
                static_cast<unsigned>(queries.size()),
                std::max(1u, std::thread::hardware_concurrency())
        );
        std::mutex result_mutex;
        std::vector <std::jthread> workers;
        workers.reserve(num_workers);

        // Каждый поток пишет только в results[i] для своих запросов; i совпадает с индексом запроса
        for (unsigned t = 0; t < num_workers; ++t) {
            workers.emplace_back([this, &queries, &results, &result_mutex, max_responses, num_workers, t] {
                std::vector <RelativeIndex> local_list;
                local_list.reserve(512);
                const auto indices = std::views::iota(static_cast<size_t>(t), queries.size()) |
                                     std::views::stride(static_cast<size_t>(num_workers));
                for (size_t i: indices) {
                    process_one_query(index_, queries[i], max_responses, max_word_length_, local_list);
                    {
                        std::lock_guard lock(result_mutex);
                        results[i] = std::move(local_list);
                    }
                }
            });
        }
        workers.clear();

        return results;
    }

} // namespace fulltext_search_service

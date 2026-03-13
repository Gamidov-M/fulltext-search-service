#include "fuzzy.hpp"
#include <algorithm>
#include <vector>

namespace fulltext_search_service {

    int levenshtein_distance(std::string_view a, std::string_view b) {
        const size_t n = a.size();
        const size_t m = b.size();
        if (n == 0) {
            return static_cast<int>(m);
        }

        if (m == 0) {
            return static_cast<int>(n);
        }

        // Два ряда: текущий и предыдущий (экономим память)
        std::vector<int> prev(m + 1);
        std::vector<int> curr(m + 1);

        for (size_t j = 0; j <= m; ++j) {
            prev[j] = static_cast<int>(j);
        }

        for (size_t i = 1; i <= n; ++i) {
            curr[0] = static_cast<int>(i);
            for (size_t j = 1; j <= m; ++j) {
                int cost = (a[i - 1] == b[j - 1]) ? 0 : 1;
                curr[j] = std::min({
                    curr[j - 1] + 1,
                    prev[j] + 1,
                    prev[j - 1] + cost
                });
            }
            std::swap(prev, curr);
        }

        return prev[m];
    }

    void find_similar_terms(
            std::string_view word,
            int max_edits,
            const std::function<void(const std::function<void(std::string_view)> &)> &iterate_terms,
            std::vector<FuzzyMatch> &out
    ) {
        out.clear();
        if (max_edits < 0) {
            return;
        }

        const size_t word_len = word.size();
        const int word_len_i = static_cast<int>(word_len);

        iterate_terms([&](std::string_view term) {
            if (term == word) {
                return;
            }

            const size_t term_len = term.size();
            if (term_len > 0 && static_cast<int>(term_len) - word_len_i > max_edits) {
                return;
            }

            if (word_len_i - static_cast<int>(term_len) > max_edits) {
                return;
            }

            int d = levenshtein_distance(word, term);
            if (d <= max_edits) {
                out.push_back({std::string(term), d});
            }
        });

        std::ranges::sort(out, [](const FuzzyMatch &x, const FuzzyMatch &y) {
            if (x.distance != y.distance) {
                return x.distance < y.distance;
            }

            return x.term < y.term;
        });
    }

} // namespace fulltext_search_service

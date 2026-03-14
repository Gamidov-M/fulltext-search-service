#include "stop_words.hpp"
#include "stemmer.hpp"
#include "utils.hpp"
#include <cctype>
#include <filesystem>
#include <fstream>
#include <string_view>

namespace fulltext_search_service {

    namespace {
        void trimInPlace(std::string &s) {
            while (!s.empty() && std::isspace(static_cast<unsigned char>(s.front()))) {
                s.erase(s.begin());
            }

            while (!s.empty() && std::isspace(static_cast<unsigned char>(s.back()))) {
                s.pop_back();
            }
        }

        void addNormalizedStopWord(
            std::unordered_set<std::string> &out,
            std::string word,
            const Stemmer *stemmer
        ) {
            trimInPlace(word);
            if (word.empty() || word.front() == '#') {
                return;
            }

            ToLowerUtf8(word);
            std::string key = stemmer ? stemmer->normalize(word) : word;
            if (key.empty()) {
                key = word;
            }

            if (!key.empty()) {
                out.insert(std::move(key));
            }
        }

        void loadLinesFromFile(
            const std::filesystem::path &path,
            std::unordered_set<std::string> &out,
            const Stemmer *stemmer,
            bool dev_mode
        ) {
            std::ifstream in(path);
            if (!in) {
                Log(dev_mode, "[dev] stop_words: не удалось открыть файл {}", path.string());
                return;
            }
            std::string line;
            while (std::getline(in, line)) {
                addNormalizedStopWord(out, std::move(line), stemmer);
            }
        }

    } // namespace

    std::shared_ptr<const std::unordered_set<std::string>> LoadStopWordsSet(
        const IndexConfig &index,
        std::string_view config_file_path,
        bool dev_mode
    ) {
        const bool have_file = !index.stop_words_file.empty();
        const bool have_inline = !index.stop_words.empty();
        if (!have_file && !have_inline) {
            return nullptr;
        }

        std::unique_ptr<Stemmer> stemmer;
        if (index.stemming_enabled && !index.stemming_language.empty()) {
            stemmer = Stemmer::create(index.stemming_language.c_str(), nullptr);
            if (!stemmer) {
                Log(dev_mode, "[dev] stop_words: стеммер не создан, стоп-слова без стемминга");
            }
        }
        const Stemmer *stem_ptr = stemmer.get();

        auto set_mut = std::make_shared<std::unordered_set<std::string>>();
        set_mut->reserve(256);

        if (have_file) {
            namespace fs = std::filesystem;
            fs::path p(index.stop_words_file);
            if (p.is_relative() && !config_file_path.empty()) {
                const fs::path base = fs::path(config_file_path).parent_path();
                if (!base.empty()) {
                    p = base / p;
                }
            }

            loadLinesFromFile(p, *set_mut, stem_ptr, dev_mode);
        }

        for (const std::string &w : index.stop_words) {
            addNormalizedStopWord(*set_mut, w, stem_ptr);
        }

        if (set_mut->empty()) {
            return nullptr;
        }

        return set_mut;
    }

} // namespace fulltext_search_service

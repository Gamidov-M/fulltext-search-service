#include "stemmer.hpp"
#include <libstemmer.h>
#include <string>
#include <unordered_map>

namespace fulltext_search_service {

    struct Stemmer::Impl {
        sb_stemmer *stemmer = nullptr;

        explicit Impl(sb_stemmer *s) : stemmer(s) {}

        ~Impl() {
            if (stemmer) {
                sb_stemmer_delete(stemmer);
                stemmer = nullptr;
            }
        }

        Impl(Impl &&) = delete;
        Impl &operator=(Impl &&) = delete;
    };

    static constexpr const char *kUtf8Encoding = "UTF_8";

    static std::string makeTlsKey(const std::string &algorithm, const std::string &encoding) {
        return algorithm + '\0' + encoding;
    }

    std::unique_ptr<Stemmer> Stemmer::create(const char *algorithm, const char *charenc) {
        if (!algorithm || !*algorithm) {
            return nullptr;
        }
        const char *enc = charenc ? charenc : kUtf8Encoding;
        sb_stemmer *s = sb_stemmer_new(algorithm, enc);
        if (!s) {
            return nullptr;
        }
        sb_stemmer_delete(s);
        return std::unique_ptr<Stemmer>(new Stemmer(algorithm, enc));
    }

    Stemmer::Stemmer(std::string algorithm, std::string encoding)
        : algorithm_(std::move(algorithm)), encoding_(std::move(encoding)) {}

    Stemmer::~Stemmer() = default;

    std::string Stemmer::normalize(const std::string &word) const {
        if (word.empty() || algorithm_.empty()) {
            return word;
        }

        thread_local std::unordered_map<std::string, std::unique_ptr<Impl>> tls_cache;
        std::string key = makeTlsKey(algorithm_, encoding_);
        auto it = tls_cache.find(key);
        if (it == tls_cache.end()) {
            sb_stemmer *s = sb_stemmer_new(algorithm_.c_str(), encoding_.c_str());
            if (!s) {
                return word;
            }
            it = tls_cache.emplace(std::move(key), std::make_unique<Impl>(s)).first;
        }

        Impl *impl = it->second.get();
        if (!impl || !impl->stemmer) {
            return word;
        }

        const sb_symbol *input = reinterpret_cast<const sb_symbol *>(word.data());
        int size = static_cast<int>(word.size());
        const sb_symbol *out = sb_stemmer_stem(impl->stemmer, input, size);
        if (!out) {
            return word;
        }

        int len = sb_stemmer_length(impl->stemmer);
        return std::string(reinterpret_cast<const char *>(out), static_cast<size_t>(len));
    }

} // namespace fulltext_search_service

#include <chrono>
#include <cstdlib>
#include <httplib.h>
#include <nlohmann/json.hpp>
#include <print>
#include <random>
#include <string>
#include <vector>

namespace {

    using json = nlohmann::json;

    const std::vector <std::string> kWords = {
            "программа", "данные", "поиск", "индекс", "документ", "сервер", "запрос", "результат", "скорость", "тест",
            "система", "файл", "база", "код", "алгоритм", "модуль", "интерфейс", "клиент", "сеть", "память", "москва",
            "россия", "столица", "город", "страна", "история", "культура"
    };

    std::string randomSentence(std::mt19937 &rng, int wordCount) {
        std::uniform_int_distribution <size_t> dist(0, kWords.size() - 1);
        std::string s;
        for (int i = 0; i < wordCount; ++i) {
            if (!s.empty()) {
                s += ' ';
            }
            s += kWords[dist(rng)];
        }

        return s;
    }

    json makeDocumentsBatch(int startId, int count, std::mt19937 &rng) {
        json arr = json::array();
        std::uniform_int_distribution<int> wordsPerDoc(5, 25);
        for (int i = 0; i < count; ++i) {
            arr.push_back({
                {"content", randomSentence(rng, wordsPerDoc(rng))}
            });
        }

        return arr;
    }

    bool postDocuments(httplib::Client &cli, const json &docs, int &received) {
        auto res = cli.Post("/indexes/documents", docs.dump(), "application/json");
        if (!res || res->status != 202) {
            return false;
        }
        json body = json::parse(res->body.empty() ? "{}" : res->body);
        received = body.value("received", 0);

        return true;
    }

    bool doSearch(httplib::Client &cli, const std::string &query, int limit, std::vector <int64_t> &outTimeMs) {
        json body = {
                {"q",     query},
                {"limit", limit}
        };
        auto t0 = std::chrono::steady_clock::now();
        auto res = cli.Post("/indexes/default/search", body.dump(), "application/json");
        auto t1 = std::chrono::steady_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
        outTimeMs.push_back(static_cast<int64_t>(ms));
        if (!res || res->status != 200) {
            return false;
        }

        return true;
    }

} // namespace

int main(int argc, char *argv[]) {
    int totalDocs = 50'000;
    int batchSize = 1000;
    if (argc >= 2) {
        totalDocs = std::max(1, std::atoi(argv[1]));
    }
    if (argc >= 3) {
        batchSize = std::max(1, std::min(5000, std::atoi(argv[2])));
    }

    std::println("Бенчмарк загрузки и поиска \n");

    std::println("Параметры: документов = {}, размер батча = {}\n", totalDocs, batchSize);

    httplib::Client cli("127.0.0.1", 8000);
    cli.set_connection_timeout(2, 0);

    std::mt19937 rng(12345);

    std::println("1 -- Загрузка {} документов (батчами по {})...", totalDocs, batchSize);

    auto tIndexStart = std::chrono::steady_clock::now();
    int loaded = 0;
    int batchCount = 0;
    for (int offset = 0; offset < totalDocs;) {
        int count = std::min(batchSize, totalDocs - offset);
        json batch = makeDocumentsBatch(offset, count, rng);
        int received = 0;
        if (!postDocuments(cli, batch, received)) {
            std::println(stderr, "Ошибка загрузки батча (offset={})", offset);
            return 1;
        }

        loaded += received;
        ++batchCount;
        offset += count;
        if (batchCount % 10 == 0 || offset >= totalDocs) {
            std::println("   загружено {} / {}", loaded, totalDocs);
        }
    }

    auto tIndexEnd = std::chrono::steady_clock::now();
    auto indexMs = std::chrono::duration_cast<std::chrono::milliseconds>(tIndexEnd - tIndexStart).count();
    double indexSec = indexMs / 1000.0;
    std::println("   Итого: {} документов за {:.2f} с ({:.0f} док/с)\n", loaded, indexSec, loaded / indexSec);

    std::println("2 -- Поиск: 20 запросов по одному и по два слова...");

    std::vector <std::string> queries = {
            "поиск", "документ", "индекс", "сервер", "данные", "москва", "россия",
            "программа код", "база данные", "поиск результат", "индекс документ",
    };
    std::vector <int64_t> searchTimesMs;
    searchTimesMs.reserve(20);
    for (int i = 0; i < 20; ++i) {
        const std::string &q = queries[i % queries.size()];
        if (!doSearch(cli, q, 10, searchTimesMs)) {
            std::println(stderr, "Ошибка поиска по запросу \"{}\"", q);
            return 1;
        }
    }

    int64_t sumMs = 0, minMs = searchTimesMs.empty() ? 0 : searchTimesMs[0], maxMs = 0;
    for (int64_t ms: searchTimesMs) {
        sumMs += ms;
        minMs = std::min(minMs, ms);
        maxMs = std::max(maxMs, ms);
    }
    double avgMs = searchTimesMs.empty() ? 0 : static_cast<double>(sumMs) / searchTimesMs.size();
    std::println(
            "   Запросов: {}, мин = {} мс, макс = {} мс, среднее = {:.2f} мс\n",
            static_cast<int>(searchTimesMs.size()), minMs, maxMs, avgMs
    );

    return 0;
}

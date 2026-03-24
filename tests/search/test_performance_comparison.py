import aiohttp
import asyncio
import time
from typing import Dict, Any, List


class SearchTester:
    def __init__(self):
        self.base_url = "http://127.0.0.1:8001"
        self.collection = "dissertations"
        self.url = f"{self.base_url}/indexes/{self.collection}/search"
        self.headers = {"Content-Type": "application/json"}

    async def _execute_search(self, session: aiohttp.ClientSession, query_params: Dict[str, Any],
                              request_name: str = "запрос", parallel_counts: List[int] = None):

        if parallel_counts is None:
            parallel_counts = [1, 10, 50, 100]

        print(f"\n{'=' * 60}")
        print(f" ТЕСТ: {request_name}")
        print(f" Параметры: {query_params}")
        print(f"{'=' * 60}")

        results = {}

        for count in parallel_counts:
            print(f"\n Тестируем {count} параллельных запросов...")

            start = time.time()

            tasks = [session.post(self.url, headers=self.headers, json=query_params)
                     for _ in range(count)]

            responses = await asyncio.gather(*tasks)

            for response in responses:
                await response.json()

            total_time = time.time() - start

            results[count] = {
                "total_time_ms": total_time * 1000,
                "avg_time_per_request_ms": (total_time / count) * 1000,
                "requests_per_second": count / total_time
            }

            print(f" {count} запросов выполнено за {total_time * 1000:.2f}мс")
            print(f" Среднее на запрос: {(total_time / count) * 1000:.2f}мс")
            print(f" RPS (запросов/сек): {count / total_time:.2f}")

        baseline = results.get(1, {}).get("total_time_ms", 1)

        print(f"\n СРАВНИТЕЛЬНЫЙ АНАЛИЗ для {request_name}:")
        print(f"{'-' * 50}")
        print(f"Кол-во | Общее время (мс) | Среднее (мс) | Ускорение | RPS")
        print(f"{'-' * 50}")

        for count in parallel_counts:
            if count in results:
                total_time = results[count]["total_time_ms"]
                avg_time = results[count]["avg_time_per_request_ms"]
                rps = results[count]["requests_per_second"]

                if baseline and baseline > 0:
                    speedup = baseline / total_time
                else:
                    speedup = 0

                print(f"{count:4d} | {total_time:12.2f} | {avg_time:10.2f} | {speedup:8.2f}x | {rps:7.2f}")

        print(f"{'-' * 50}")

        return results

    async def test_all(self):
        async with aiohttp.ClientSession() as session:
            # Тест 1: Поиск стандартный
            await self._execute_search(
                session,
                {"q": "имени", "limit": 5},
                "Search Basic"
            )
            # Тест 2: Поиск с crop
            await self._execute_search(
                session,
                {
                    "q": "второй",
                    "crop_fields": ["content"],
                    "crop_length": 5
                 },
                "Search with crop"
            )

            # Тест 3: Поиск с crop и highlight
            await self._execute_search(
                session,
                {
                    "q": "второй",
                    "crop_fields": ["content"],
                    "crop_length": 5,
                    "highlight": True
                },
                "Search with crop and highlight"
            )

            # Тест 4: Поиск с crop и marker
            await self._execute_search(
                session,
                {
                    "q": "имени",
                    "crop_fields": ["name"],
                    "crop_length": 5,
                    "crop_marker": "***"
                },
                "Search with crop and marker"
            )

            # Тест 5: Поиск с phrase
            await self._execute_search(
                session,
                {"q": "имени", "limit": 5, "phrase": True},
                "Search with phrase"
            )

            # Тест 6: Поиск с field selector
            await self._execute_search(
                session,
                {"q": "хранившихся", "attributesToRetrieve": ["id", "title"]},
                "Search with attributesToRetrieve"
            )

            # Тест 7: Поиск с fuzzy
            await self._execute_search(
                session,
                {"q": "трихугольную", "limit": 5, "fuzzy": True},
                "Search with fuzzy"
            )

            # Тест 8: Поиск с fuzzy with edit
            await self._execute_search(
                session,
                {"q": "ЦЕТАТА", "limit": 5, "fuzzy": True, "fuzzy_max_edits": 1},
                "Search with fuzzy and fuzzy_max,edits"
            )

            # Тест 9: Поиск с partial
            await self._execute_search(
                session,
                {"q": "док", "limit": 5, "partial": True},
                "Search with partial"
            )

            # Тест 10: Поиск с highlight
            await self._execute_search(
                session,
                {"q": "Второй", "limit": 5, "offset": 0, "highlight": True},
                "Search with highlight"
            )


if __name__ == "__main__":
    print(" ЗАПУСК НАГРУЗОЧНОГО ТЕСТИРОВАНИЯ")
    print(f" URL: http://127.0.0.1:8001/indexes/dissertations/search")
    print(f"⚙  Тестируем: 1, 10, 50, 100 параллельных запросов")
    print(f"{'=' * 60}")

    tester = SearchTester()
    asyncio.run(tester.test_all())

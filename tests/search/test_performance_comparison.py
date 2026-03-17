import aiohttp
import asyncio
import time
from typing import Dict, Any


class SearchTester:
    def __init__(self):
        self.url = "http://127.0.0.1:8001/indexes/dissertations/search"
        self.headers = {"Content-Type": "application/json"}

    async def _execute_search(self, session: aiohttp.ClientSession, query_params: Dict[str, Any],
                              request_name: str = "запрос"):
        start = time.time()
        async with session.post(self.url, headers=self.headers, json=query_params) as resp:
            await resp.json()
        single_time = time.time() - start
        print(f"1 {request_name}: {single_time * 1000:.2f} мс")

        start = time.time()
        tasks = [session.post(self.url, headers=self.headers, json=query_params) for _ in range(10)]
        responses = await asyncio.gather(*tasks)
        for r in responses:
            await r.json()
        parallel_time = time.time() - start
        print(f"10 параллельных {request_name}: {parallel_time * 1000:.2f} мс")

        # Сравнение
        print(f"\n📊 Ускорение: {single_time * 10 / parallel_time:.1f}x")
        print(f"Среднее на запрос при параллельных: {(parallel_time / 10) * 1000:.2f}мс")
        print("-" * 50)

        return single_time, parallel_time

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


tester = SearchTester()
asyncio.run(tester.test_all())

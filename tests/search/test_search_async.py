import asyncio
import aiohttp
import time


class AsyncSearchTester:
    def __init__(self):
        self.base_url = "http://127.0.0.1:8001"
        self.collection = "dissertations"
        self.url = f"{self.base_url}/indexes/{self.collection}/search"
        self.headers = {"Content-Type": "application/json"}

    async def run_test(self, session, name, payload):
        try:
            start = time.time()
            async with session.post(self.url, headers=self.headers, json=payload) as resp:
                data = await resp.json()
                duration = (time.time() - start) * 1000

                print(f"\n▶ {name}")
                print(f"  Статус: {resp.status}")
                print(f"  Время: {duration:.2f}мс")

                total = data.get('total', 'Не найдено')

                return {
                    'name': name,
                    'status': resp.status,
                    'time': duration,
                    'success': resp.status == 200,
                    'total': total
                }
        except Exception as e:
            print(f"\n▶ {name}\n  ❌ Ошибка: {e}")
            return {'name': name, 'success': False, 'error': str(e)}

    async def test_parallel_queries(self, session):
        queries = [
            ("Базовый поиск 'хранившихся'", {"q": "имени", "limit": 5}),
            ("Обрезка полей", {"q": "второй", "crop_fields": ["content"], "crop_length": 5}),
            ("Обрезка+выделение", {"q": "второй", "crop_fields": ["content"], "crop_length": 5, "highlight": True}),
            ("Обрезка с маркером", {"q": "имени", "crop_fields": ["name"], "crop_length": 5, "crop_marker": "***"}),
            ("Точная фраза", {"q": "имени", "phrase": True}),
            ("Массив полей", {"q": "хранившихся", "attributesToRetrieve": ["id", "title"]}),
            ("Нечеткий 'ЦЕТАТА'", {"q": "трихугольную", "fuzzy": True}),
            ("Нечеткий с правками", {"q": "ЦЕТАТА", "fuzzy": True, "fuzzy_max_edits": 1}),
            ("Частичный поиск", {"q": "док", "partial": True}),
            ("Поиск с выделением", {"q": "Второй", "highlight": True}),
        ]

        tasks = []
        for i, (q, params) in enumerate(queries):
            name = f"Параллельный #{i + 1}: '{q}'"
            payload = {"q": q, **params}
            tasks.append(self.run_test(session, name, payload))

        results = await asyncio.gather(*tasks)
        return results

    async def test_load_50_queries(self, session):
        base_queries = [
            ("Базовый поиск 'хранившихся'", {"q": "имени", "limit": 5}),
            ("Обрезка полей", {"q": "второй", "crop_fields": ["content"], "crop_length": 5}),
            ("Обрезка+выделение", {"q": "второй", "crop_fields": ["content"], "crop_length": 5, "highlight": True}),
            ("Обрезка с маркером", {"q": "имени", "crop_fields": ["name"], "crop_length": 5, "crop_marker": "***"}),
            ("Точная фраза", {"q": "имени", "phrase": True}),
            ("Массив полей", {"q": "хранившихся", "attributesToRetrieve": ["id", "title"]}),
            ("Нечеткий 'ЦЕТАТА'", {"q": "трихугольную", "fuzzy": True}),
            ("Нечеткий с правками", {"q": "ЦЕТАТА", "fuzzy": True, "fuzzy_max_edits": 1}),
            ("Частичный поиск", {"q": "док", "partial": True}),
            ("Поиск с выделением", {"q": "Второй", "highlight": True})
                  ] * 10

        start = time.time()
        tasks = []
        for i, (name, params) in enumerate(base_queries):
            q = name.split("'")[1] if "'" in name else name.split()[-1].lower()
            tasks.append(self.run_test(session, f"Нагрузка #{i + 1}: {name[:20]}",
                                       {"q": q, "limit": 5, **params}))

        results = await asyncio.gather(*tasks)
        total_time = (time.time() - start) * 1000

        success = sum(1 for r in results if r.get('success', False))

        print(f"\n Нагрузка 50 запросов:")
        print(f"  Всего: 50 запросов")
        print(f"  Успешно: {success}")
        print(f"  Общее время: {total_time:.2f}мс")
        print(f"  Среднее: {total_time / 50:.2f}мс")
        print(f"  Запросов/сек: {50 / (total_time / 1000):.2f}")

        return results

    async def run_all(self):
        print("\n" + "=" * 60)
        print("⚡ АСИНХРОННЫЕ ТЕСТЫ (aiohttp)")
        print("=" * 60)

        async with aiohttp.ClientSession() as session:
            tests = [
                ("10 параллельных запросов", self.test_parallel_queries(session)),
                ("Нагрузка 50 запросов", self.test_load_50_queries(session)),
            ]

            all_results = []
            for name, coro in tests:
                print(f"\n{name}")
                results = await coro
                all_results.extend(results if isinstance(results, list) else [results])

            success = sum(1 for r in all_results if r.get('success', False))
            print(f"\n<<< Итоги асинхронных тестов: {success}/{len(all_results)} успешно >>>")


async def main():
    tester = AsyncSearchTester()
    await tester.run_all()


if __name__ == "__main__":
    asyncio.run(main())

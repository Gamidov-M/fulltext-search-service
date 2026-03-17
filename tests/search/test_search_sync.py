import time
import requests


class SearchTester:
    def __init__(self):
        self.base_url = "http://127.0.0.1:8001"
        self.collection = "dissertations"
        self.url = f"{self.base_url}/indexes/{self.collection}/search"
        self.headers = {"Content-Type": "application/json"}

    def run_test(self, name, payload):
        try:
            start_time = time.time()

            response = requests.post(self.url, headers=self.headers, json=payload)

            end_time = time.time()

            request_time = end_time - start_time

            print(f"\nТест: {name}")
            print(f"Status Code: {response.status_code}")
            print(f"Время выполнения запроса: {request_time * 1000:.2f} миллисекунд")

            return {
                'name': name,
                'status': response.status_code,
                'time': request_time,
                'success': response.status_code == 200
            }
        except requests.exceptions.RequestException as e:
            print(f"Ошибка запроса: {e}")

    def test_basic_search(self):
        """Тест 1: Базовый поиск"""
        return self.run_test(
            "Базовый поиск 'хранившихся'",
            {"q": "имени", "limit": 5, "offset": 0}
        )

    def test_search_crop(self):
        """Тест 2: Обрезка полей"""
        return self.run_test(
            "Обрезка полей",
            {"q": "второй", "crop_fields": ["content"], "crop_length": 5}
        )

    def test_search_crop_and_highlight(self):
        """Тест 3: Обрезка полей и выделение"""
        return self.run_test(
            "Обрезка полей и выделение",
            {"q": "второй", "crop_fields": ["content"], "crop_length": 5, "highlight": True}
        )

    def test_search_crop_marker(self):
        """Тест 4: Обрезка полей в нач./конце"""
        return self.run_test(
            "Обрезка полей в нач./конце",
            {"q": "имени", "crop_fields": ["name"], "crop_length": 5, "crop_marker": "***"}
        )

    def test_search_exact_phrase(self):
        """Тест 4: Точная фраза"""
        return self.run_test(
            "Точная фраза",
            {"q": "имени", "limit": 5, "phrase": True}
        )

    def test_search_field_selector(self):
        """Тест 5: Массив имён полей"""
        return self.run_test(
            "Массив имён полей",
            {"q": "хранившихся", "attributesToRetrieve": ["id", "title"]}
        )

    def test_search_fuzzy(self):
        """Тест 6: Нечеткий поиск"""
        return self.run_test(
            "Нечеткий поиск 'ЦЕТАТА",
            {"q": "трихугольную", "limit": 5, "fuzzy": True}
        )

    def test_search_fuzzy_with_edit(self):
        """Тест 7: Нечеткий поиск c указанным кол-вом"""
        return self.run_test(
            "Нечеткий поиск c указанным кол-вом",
            {"q": "ЦЕТАТА", "limit": 5, "fuzzy": True, "fuzzy_max_edits": 1}
        )

    def test_search_partial(self):
        """Тест 8: Поиск из любой части слова"""
        return self.run_test(
            "Поиск из любой части слова",
            {"q": "док", "limit": 5, "partial": True}
        )

    def test_search_with_highlight(self):
        """Тест 9: Поиск с выделением"""
        return self.run_test(
            " Поиск с выделением",
            {"q": "Второй", "limit": 5, "offset": 0, "highlight": True}
        )

    def run_all(self):
        """Запуск всех синхронных тестов"""
        print("\n" + "=" * 60)
        print("СИНХРОННЫЕ ТЕСТЫ (requests)")
        print("=" * 60)

        tests = [
            self.test_basic_search,
            self.test_search_crop,
            self.test_search_crop_and_highlight,
            self.test_search_crop_marker,
            self.test_search_exact_phrase,
            self.test_search_field_selector,
            self.test_search_fuzzy,
            self.test_search_fuzzy_with_edit,
            self.test_search_partial,
            self.test_search_with_highlight,
        ]

        results = []
        for test in tests:
            results.append(test())

        success = sum(1 for r in results if r['success'])
        print(f"<<< Итоги синхронных тестов: {success}/{len(results)} успешно >>>")
        return results


searchTester = SearchTester()

searchTester.run_all()

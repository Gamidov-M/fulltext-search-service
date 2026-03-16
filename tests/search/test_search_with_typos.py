import time

import requests


class SearchTester:
    def __init__(self):
        self.base_url = "http://127.0.0.1:8001"
        self.collection = "dissertations"
        self.url = f"{self.base_url}/indexes/{self.collection}/search"
        self.headers = {"Content-Type": "application/json"}

    def run_test(self):
        payload = {"q": "имни", "limit": 5, "fuzzy": True, "fuzzy_max_edits": 2}  # Поиск с опечатками

        try:
            start_time = time.time()

            response = requests.post(self.url, headers=self.headers, json=payload)

            end_time = time.time()

            request_time = end_time - start_time

            print(f"Status Code: {response.status_code}")
            print(f"Response: {response.json()}")
            print(f"Время выполнения запроса: {request_time * 1000:.2f} миллисекунд")
        except requests.exceptions.RequestException as e:
            print(f"Ошибка запроса: {e}")


searchTester = SearchTester()

searchTester.run_test()
